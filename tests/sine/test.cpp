/**
 * Just a simple test program where we encode a stereo sine tone and then
 * decode it again and display the result.
 */

#include <assert.h>
#include <iostream>
#include <string.h>
#include "ADPCM.h"
#include "ADPCMVector.h"
#include "SineGenerator.h"

using namespace adpcm_ffmpeg;

AVCodecID code =
    AV_CODEC_ID_ADPCM_MS;  // AV_CODEC_ID_ADPCM_MS; // AV_CODEC_ID_ADPCM_IMA_WAV
ADPCMVector<int16_t> sample_vector;
SineWaveGenerator<int16_t> genLeft{30000.0};
SineWaveGenerator<int16_t> genRight{30000.0};
int channels = 2;
int sample_rate = 44100;
int loop_count = 100;

int loadSamples(int frame_size) {
  // fill frame with data
  int result = 0;
  for (int j = 0; j < frame_size; j += channels) {
    sample_vector[j] = genLeft.nextSample();
    if (channels == 2) sample_vector[j + 1] = genRight.nextSample();
    result += channels;
  }
  return result;
}

void displayPacket(AVPacket& packet) {
  for (int j = 0; j < packet.size; j++) {
    if (j % 16 == 0) std::cout << std::endl;
    printf("0x%x ", packet.data[j]);
  }
  std::cout << std::endl;
}

void displayResult(AVFrame& frame) {
  // print the result
  int16_t* data = (int16_t*)frame.data[0];
  size_t samples = frame.nb_samples * channels ;
  for (int j = 0; j < samples; j += channels) {
    for (int ch = 0; ch < channels; ch++) {
      std::cout << data[j + ch] << " ";
    }
    std::cout << std::endl;
  }
}

int checkFrameSize(ADPCMDecoder &decoder, ADPCMEncoder &encoder) {
  // determine frame size
  size_t frame_size = encoder.frameSize();
  assert(frame_size == decoder.frameSize());
  assert(frame_size > 0);
  return frame_size;
}

void test(AVCodecID id, const char *title) {
  std::cout << title << "\n";
  ADPCMDecoder& decoder = *ADPCMDecoderFactory::create(id);
  ADPCMEncoder& encoder = *ADPCMEncoderFactory::create(id);

  assert(&decoder != nullptr);
  assert(&encoder != nullptr);

  genLeft.begin(sample_rate, 220);
  genRight.begin(sample_rate, 440);

  // open codec
  if (!encoder.begin(sample_rate, channels)) {
    std::cout << "encoder not supported";
    return;
  }
  if (!decoder.begin(sample_rate, channels)) {
    std::cout << "decoder not supported";
    return;
  }

  int frame_size = checkFrameSize(decoder, encoder);
  int sample_count = frame_size * channels;
  int frame_count = sample_count;

  // setup data for frame
  sample_vector.resize(sample_count);

  for (int n = 0; n < loop_count; n++) {
    size_t samples = loadSamples(sample_count);

    AVPacket& packet = encoder.encode(&sample_vector[0], sample_count);
    displayPacket(packet);

    // decode
    AVFrame& frame = decoder.decode(packet);
    displayResult(frame);
  }
  // close
  encoder.end();
  decoder.end();
  delete &encoder;
  delete &decoder;
}

/// Test encode/decode round-trip using 8-bit PCM (int8_t and uint8_t).
void test8bit(AVCodecID id, const char *title) {
  std::cout << "8-bit test: " << title << "\n";

  // signed 8-bit encode test
  {
    ADPCMDecoder& decoder = *ADPCMDecoderFactory::create(id);
    ADPCMEncoder& encoder = *ADPCMEncoderFactory::create(id);

    SineWaveGenerator<int8_t> genL8{100.0};
    SineWaveGenerator<int8_t> genR8{100.0};
    genL8.begin(sample_rate, 220);
    genR8.begin(sample_rate, 440);

    if (!encoder.begin(sample_rate, channels)) {
      std::cout << "  encoder not supported\n";
      delete &encoder;
      delete &decoder;
      return;
    }
    if (!decoder.begin(sample_rate, channels)) {
      std::cout << "  decoder not supported\n";
      delete &encoder;
      delete &decoder;
      return;
    }

    int frame_size = encoder.frameSize();
    int sample_count = frame_size * channels;
    ADPCMVector<int8_t> samples8;
    samples8.resize(sample_count);
    ADPCMVector<int8_t> decoded8;
    decoded8.resize(sample_count);

    for (int n = 0; n < 3; n++) {
      for (int j = 0; j < sample_count; j += channels) {
        samples8[j] = genL8.nextSample();
        if (channels == 2) samples8[j + 1] = genR8.nextSample();
      }
      AVPacket& packet = encoder.encode(&samples8[0], sample_count);
      assert(packet.size > 0);
      AVFrame& frame = decoder.decode(packet);
      assert(frame.nb_samples > 0);
      decoder.toInt8(frame, &decoded8[0]);
      std::cout << "  int8 in=" << (int)samples8[0] << " out=" << (int)decoded8[0] << "\n";
    }
    encoder.end();
    decoder.end();
    delete &encoder;
    delete &decoder;
  }

  // unsigned 8-bit encode test
  {
    ADPCMDecoder& decoder = *ADPCMDecoderFactory::create(id);
    ADPCMEncoder& encoder = *ADPCMEncoderFactory::create(id);

    SineWaveGenerator<int8_t> genL8{100.0};
    SineWaveGenerator<int8_t> genR8{100.0};
    genL8.begin(sample_rate, 220);
    genR8.begin(sample_rate, 440);

    encoder.begin(sample_rate, channels);
    decoder.begin(sample_rate, channels);

    int frame_size = encoder.frameSize();
    int sample_count = frame_size * channels;
    ADPCMVector<uint8_t> samplesU8(sample_count);
    ADPCMVector<uint8_t> decodedU8(sample_count);

    for (int n = 0; n < 3; n++) {
      for (int j = 0; j < sample_count; j += channels) {
        samplesU8[j] = (uint8_t)(genL8.nextSample() + 128);
        if (channels == 2) samplesU8[j + 1] = (uint8_t)(genR8.nextSample() + 128);
      }
      AVPacket& packet = encoder.encode(&samplesU8[0], sample_count);
      assert(packet.size > 0);
      AVFrame& frame = decoder.decode(packet);
      assert(frame.nb_samples > 0);
      decoder.toUInt8(frame, &decodedU8[0]);
      std::cout << "  uint8 in=" << (int)samplesU8[0] << " out=" << (int)decodedU8[0] << "\n";
    }
    encoder.end();
    decoder.end();
    delete &encoder;
    delete &decoder;
  }
}


int main() {
  test(AV_CODEC_ID_ADPCM_IMA_WAV,"IMA_WAV");
  test(AV_CODEC_ID_ADPCM_IMA_SSI,"IMA_SSI");
  test(AV_CODEC_ID_ADPCM_IMA_ALP, "IMA_ALP");
  test(AV_CODEC_ID_ADPCM_MS, "MS");
  test(AV_CODEC_ID_ADPCM_YAMAHA,"YAHMA");
  test(AV_CODEC_ID_ADPCM_IMA_APM,"IMA_APM");
  test(AV_CODEC_ID_ADPCM_ARGO,"ARGO");
  test(AV_CODEC_ID_ADPCM_IMA_WS,"IMA_WS");
  test(AV_CODEC_ID_ADPCM_SWF,"SWF");
  // test(AV_CODEC_ID_ADPCM_IMA_AMV,"IMA_AMV"); // only mono at 22050!
  // test(AV_CODEC_ID_ADPCM_IMA_QT,"IMA_QT"); // broken !

  // 8-bit PCM round-trip tests
  test8bit(AV_CODEC_ID_ADPCM_IMA_WAV, "IMA_WAV");
  test8bit(AV_CODEC_ID_ADPCM_MS, "MS");
  test8bit(AV_CODEC_ID_ADPCM_YAMAHA, "YAMAHA");

  std::cout << "*** END ***" << "\n";


  return 0;
}