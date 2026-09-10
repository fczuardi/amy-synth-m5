#pragma once

#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

// Core Gray AMY PCM output bridge.
//
// AMY renders signed 16-bit stereo PCM blocks. The original Core Gray speaker
// path is owned by M5Unified, so this bridge keeps AMY in AMY_AUDIO_IS_NONE
// mode, renders blocks on AMY's cadence, mixes them to mono, and queues them
// into M5.Speaker.playRaw().
class AmyM5SpeakerBridge {
 public:
  void begin();

  // Render and queue at most a small catch-up burst. Apps should call this
  // frequently while sound is active or while a release tail is still audible.
  void update();

  // Stop the M5Unified output channel and discard any partially-filled buffer.
  // This is separate from AMY note state; app policy decides when silence has
  // lasted long enough to close the Core Gray speaker path.
  void stopOutput();

  bool amyStarted() const;
  uint32_t renderedBlockCount() const;
  uint32_t queuedBufferCount() const;
  uint32_t droppedBufferCount() const;
  uint32_t queueBlockedCount() const;
  uint8_t speakerQueueDepth() const;

 private:
  static constexpr uint8_t AUDIO_CHANNEL = 0;
  static constexpr int32_t OUTPUT_GAIN = 2;
  static constexpr uint32_t AMY_BLOCK_INTERVAL_US =
      (AMY_BLOCK_SIZE * 1000000UL) / AMY_SAMPLE_RATE;
  static constexpr uint8_t MAX_CATCH_UP_BLOCKS_PER_LOOP = 3;
  static constexpr size_t STREAM_BUFFER_COUNT = 3;
  static constexpr size_t AMY_BLOCKS_PER_STREAM_BUFFER = 3;
  static constexpr size_t STREAM_BUFFER_SAMPLES =
      AMY_BLOCK_SIZE * AMY_BLOCKS_PER_STREAM_BUFFER;

  void configureSpeaker();
  void startAmyForPcm();
  bool queueActiveBuffer();
  bool canRenderNextAmyBlock() const;
  int16_t mixFrameToMono(int16_t* samples, size_t frame) const;
  void appendAmyBlock(int16_t* samples);

  int16_t streamBuffers_[STREAM_BUFFER_COUNT][STREAM_BUFFER_SAMPLES] = {};

  uint32_t lastAmyRenderAtUs_ = 0;
  uint32_t renderedBlockCount_ = 0;
  uint32_t queuedBufferCount_ = 0;
  uint32_t droppedBufferCount_ = 0;
  uint32_t queueBlockedCount_ = 0;

  size_t activeBufferIndex_ = 0;
  size_t activeBufferSamples_ = 0;

  bool amyStarted_ = false;
};
