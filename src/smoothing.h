#pragma once

#define WEIGHT_HISTORY 2

float weightHistory[WEIGHT_HISTORY] = {0};
int   weightHistoryIdx = 0;
bool  weightHistoryFull = false;

float smoothedWeight(float newReading) {
  weightHistory[weightHistoryIdx] = newReading;
  weightHistoryIdx = (weightHistoryIdx + 1) % WEIGHT_HISTORY;
  if (weightHistoryIdx == 0) weightHistoryFull = true;

  int count = weightHistoryFull ? WEIGHT_HISTORY : weightHistoryIdx;
  float sum = 0;
  for (int i = 0; i < count; i++) {
    sum += weightHistory[i];
  }
  
  return sum / count;
}

void resetSmoothing() {
    for (int i = 0; i < WEIGHT_HISTORY; i++) weightHistory[i] = 0;
    weightHistoryIdx  = 0;
    weightHistoryFull = false;
}