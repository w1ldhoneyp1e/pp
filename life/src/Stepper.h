#pragma once

#include "Field.h"

#include <utility>

namespace Stepper {

std::pair<Field, double> step(const Field& field, int numThreads);

}
