#include "AmyM5SpeakerBridge.h"

void AmyM5SpeakerBridge::begin() {
  configureSpeaker();
  startAmyForPcm();
  lastAmyRenderAtUs_ = micros();
}

void AmyM5SpeakerBridge::update() {
  const uint32_t nowUs = micros();
  uint8_t catchUpBlocks = 0;

  while (amyStarted_ &&
         nowUs - lastAmyRenderAtUs_ >= AMY_BLOCK_INTERVAL_US &&
         catchUpBlocks < MAX_CATCH_UP_BLOCKS_PER_LOOP) {
    if (!canRenderNextAmyBlock()) {
      queueBlockedCount_++;
      lastAmyRenderAtUs_ += AMY_BLOCK_INTERVAL_US;
      break;
    }

    appendAmyBlock(amy_update());
    lastAmyRenderAtUs_ += AMY_BLOCK_INTERVAL_US;
    catchUpBlocks++;
  }
}

bool AmyM5SpeakerBridge::amyStarted() const {
  return amyStarted_;
}

uint32_t AmyM5SpeakerBridge::renderedBlockCount() const {
  return renderedBlockCount_;
}

uint32_t AmyM5SpeakerBridge::queuedBufferCount() const {
  return queuedBufferCount_;
}

uint32_t AmyM5SpeakerBridge::droppedBufferCount() const {
  return droppedBufferCount_;
}

uint32_t AmyM5SpeakerBridge::queueBlockedCount() const {
  return queueBlockedCount_;
}

uint8_t AmyM5SpeakerBridge::speakerQueueDepth() const {
  return M5.Speaker.isPlaying(AUDIO_CHANNEL);
}

void AmyM5SpeakerBridge::configureSpeaker() {
  auto speakerConfig = M5.Speaker.config();
  M5.Speaker.config(speakerConfig);
  M5.Speaker.begin();
  M5.Speaker.setVolume(255);
  M5.Speaker.setAllChannelVolume(255);
}

void AmyM5SpeakerBridge::startAmyForPcm() {
  amy_config_t amyConfig = amy_default_config();
  amyConfig.audio = AMY_AUDIO_IS_NONE;
  amyConfig.midi = AMY_MIDI_IS_NONE;
  amyConfig.midi_uart = -1;
  amyConfig.midi_in = -1;
  amyConfig.midi_out = -1;
  amyConfig.features.startup_bleep = 0;
  amyConfig.features.default_synths = 0;
  amyConfig.features.audio_in = 0;
  amyConfig.platform.multithread = 1;
  amyConfig.platform.multicore = 0;

  amy_start(amyConfig);
  amyStarted_ = true;
}

bool AmyM5SpeakerBridge::queueActiveBuffer() {
  if (activeBufferSamples_ == 0) {
    return true;
  }

  if (M5.Speaker.isPlaying(AUDIO_CHANNEL) >= 2) {
    return false;
  }

  const bool queued = M5.Speaker.playRaw(streamBuffers_[activeBufferIndex_],
                                         activeBufferSamples_,
                                         AMY_SAMPLE_RATE,
                                         false,
                                         1,
                                         AUDIO_CHANNEL,
                                         false);
  if (!queued) {
    droppedBufferCount_++;
    return false;
  }

  queuedBufferCount_++;
  activeBufferIndex_ = (activeBufferIndex_ + 1) % STREAM_BUFFER_COUNT;
  activeBufferSamples_ = 0;
  return true;
}

bool AmyM5SpeakerBridge::canRenderNextAmyBlock() const {
  if (activeBufferSamples_ + AMY_BLOCK_SIZE < STREAM_BUFFER_SAMPLES) {
    return true;
  }

  return M5.Speaker.isPlaying(AUDIO_CHANNEL) < 2;
}

int16_t AmyM5SpeakerBridge::mixFrameToMono(
    int16_t* samples,
    size_t frame) const {
  const int32_t left = samples[frame * AMY_NCHANS];
  const int32_t right = samples[(frame * AMY_NCHANS) + 1];
  int32_t mono = ((left + right) / 2) * OUTPUT_GAIN;

  if (mono > INT16_MAX) {
    return INT16_MAX;
  }
  if (mono < INT16_MIN) {
    return INT16_MIN;
  }
  return static_cast<int16_t>(mono);
}

void AmyM5SpeakerBridge::appendAmyBlock(int16_t* samples) {
  if (samples == nullptr) {
    return;
  }

  int16_t* output = streamBuffers_[activeBufferIndex_] + activeBufferSamples_;
  for (size_t frame = 0; frame < AMY_BLOCK_SIZE; ++frame) {
    output[frame] = mixFrameToMono(samples, frame);
  }

  activeBufferSamples_ += AMY_BLOCK_SIZE;
  renderedBlockCount_++;

  if (activeBufferSamples_ >= STREAM_BUFFER_SAMPLES && !queueActiveBuffer()) {
    droppedBufferCount_++;
    activeBufferSamples_ = 0;
  }
}
