#include "tinyml.h"

namespace {
    tflite::ErrorReporter    *error_reporter = nullptr;
    const tflite::Model      *tfl_model      = nullptr;
    tflite::MicroInterpreter *interpreter    = nullptr;
    TfLiteTensor             *input          = nullptr;
    TfLiteTensor             *output         = nullptr;

    // 16 KB arena — đủ cho Dense(16,relu)→Dense(8,relu)→Dense(3,softmax)
    constexpr int kTensorArenaSize = 16 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

static const char *LABELS[3] = {"High Risk", "Moderate Risk", "Optimal"};

void setupTinyML() {
    Serial.println("[TinyML] Initialising TensorFlow Lite Micro...");

    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    tfl_model = tflite::GetModel(cold_storage_anomaly_tflite);
    if (tfl_model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("[TinyML] Schema mismatch: %d vs %d",
                               tfl_model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        tfl_model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        error_reporter->Report("[TinyML] AllocateTensors() failed");
        return;
    }

    input  = interpreter->input(0);
    output = interpreter->output(0);

    Serial.printf("[TinyML] OK — input dims: %d, free heap: %u bytes\n",
                  input->dims->data[1], ESP.getFreeHeap());
}

void tiny_ml_task(void *pvParameters) {
    setupTinyML();

    taskQueue     *queues = &data_queues;
    taskSemaphore *sems   = &data_sems;

    static int infer_count = 0;

    while (1) {
        if (xSemaphoreTake(sems->sTinyML, pdMS_TO_TICKS(6000)) != pdPASS) {
            continue;
        }

        mlFeatures feat;
        if (xQueueReceive(queues->qTinyML, &feat, 0) != pdPASS) {
            continue;
        }

        if (input == nullptr || output == nullptr) {
            continue;
        }

        //load features vào input tensor
        input->data.f[0] = feat.temp;
        input->data.f[1] = feat.humidity;
        input->data.f[2] = feat.temp_lag_1;
        input->data.f[3] = feat.temp_lag_5;
        input->data.f[4] = feat.temp_rolling_mean_10;
        input->data.f[5] = feat.temp_rolling_std_30;
        input->data.f[6] = feat.hour;

        if (interpreter->Invoke() != kTfLiteOk) {
            error_reporter->Report("[TinyML] Invoke() failed");
            continue;
        }

        float scores[3] = {
            output->data.f[0],
            output->data.f[1],
            output->data.f[2]
        };
        int best = 0;
        for (int i = 1; i < 3; i++) {
            if (scores[i] > scores[best]) best = i;
        }

        TinyMLResult result;
        result.class_id   = best;
        result.confidence = scores[best];
        strncpy(result.label, LABELS[best], sizeof(result.label) - 1);
        result.label[sizeof(result.label) - 1] = '\0';

        Serial.printf("[TinyML] class=%d (%s) conf=%.3f | HR=%.3f MR=%.3f OPT=%.3f\n",
                      result.class_id, result.label, result.confidence,
                      scores[0], scores[1], scores[2]);

        // Gửi kết quả cho các subsystem 
        xQueueOverwrite(queues->qTinyML_Result, &result);
        xSemaphoreGive(sems->sTinyML_Out);

        // Memory diagnostics mỗi 30 lần inference
        if (++infer_count % 30 == 0) {
            Serial.printf("[TinyML] Stack HWM: %u words, free heap: %u bytes\n",
                          uxTaskGetStackHighWaterMark(NULL), ESP.getFreeHeap());
        }
    }
}
