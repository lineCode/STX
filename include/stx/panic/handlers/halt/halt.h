/**
 * @file halt.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef STX_PANIC_HANDLERS_HALT_HALT_H_
#define STX_PANIC_HANDLERS_HALT_HALT_H_

#include <atomic>

#include "stx/panic.h"

namespace stx {

/// Causes the program, or the current thread, to halt by entering an infinite
/// loop.
[[noreturn]] inline void panic_halt(
    std::string_view info,
    SourceLocation location = SourceLocation::current()) {
  (void)info;
  (void)location;

  // TODO(lamarrr): Solve potential UB here
  while (true) {
    std::atomic_signal_fence(std::memory_order_seq_cst);
  }
}
};  // namespace stx

#endif  // STX_PANIC_HANDLERS_HALT_HALT_H_