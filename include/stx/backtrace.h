/**
 * @file backtrace.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include <cstdint>

#include "stx/internal/option_result.h"

#define STX_MAX_STACK_FRAME_DEPTH 128

// MSVC and ICC typically have 1024 max symbol length
#define STX_SYMBOL_BUFFER_SIZE 1024

//!
//!
//! - Thread and signal-safe, non-allocating.
//! - Supports local unwinding only: i.e. within the current process
//!
//!
//!

namespace stx {

namespace backtrace {

enum class SignalError {
  /// An Unknown error occurred
  Unknown,
  /// `std::signal` returned `SIG_ERR`
  SigErr
};

/// Mutable type-erased view over a contiguous character container.
struct CharSpan {
  char *data;
  size_t size;
  explicit CharSpan(char *data_, size_t size_) noexcept  // NOLINT
      : data{data_}, size{size_} {}
  CharSpan(CharSpan const &) noexcept = default;
  CharSpan(CharSpan &&) noexcept = default;
  CharSpan &operator=(CharSpan const &) noexcept = default;
  CharSpan &operator=(CharSpan &&) noexcept = default;
  ~CharSpan() noexcept = default;
};

/// `Symbol` contains references to buffers and as such should not be copied nor
/// moved as a reference. Its raw data content can also be copied as a
/// `std::string`.
struct Symbol {
  /// gets the raw symbol name, the symbol is pre-demangled if possible.
  auto raw() const noexcept -> std::string_view;

  /// construct the `Symbol` object from the raw undemangled symbol name
  /// requires that `sym`'s `data` member is not a `nullptr` and is
  /// null-terminated.
  ///
  /// UNCHECKED!
  explicit Symbol(CharSpan sym) noexcept : symbol_{sym} {};

 private:
  CharSpan symbol_;
};

/// reperesents an active stack frame.
struct Frame {
  /// instruction pointer
  Option<uintptr_t> ip;
  /// address on the call stack
  Option<uintptr_t> sp;
  /// offset of the function's call-site to the callee on the instruction block.
  Option<uintptr_t> offset;
  /// function's symbol name. possibly demangled.
  Option<Symbol> symbol;

  explicit Frame() noexcept  // NOLINT
      : ip{None}, sp{None}, offset{None}, symbol{None} {}

  Frame(Frame &&) noexcept = default;
  Frame &operator=(Frame &&) noexcept = default;

  Frame(Frame const &cp) noexcept
      : ip{cp.ip.clone()},
        sp{cp.sp.clone()},
        offset{cp.offset.clone()},
        symbol{cp.symbol.clone()} {}

  Frame &operator=(Frame const &cp) noexcept {
    ip = cp.ip.clone();
    sp = cp.sp.clone();
    offset = cp.offset.clone();
    symbol = cp.symbol.clone();
    return *this;
  }

  ~Frame() noexcept = default;
};

using Callback = bool (*)(Frame, int);

/// Gets a backtrace within the current machine's state.
/// This function walks down the stack, and calls callback on each stack frame
/// as it does so. if the callback invocation's result evaluates to true, it
/// stops which means it has found the desired stack frame, else, it keeps
/// walking the stack until it exhausts the stack frame / reaches the maximum
/// stack depth. This function does not perform stack-unwinding.
///
/// Returns the number of stack frames read.
///
///
/// `callback`  is allowed to throw exceptions.
///
/// # WARNING
///
/// Do not move the frame nor its member objects out of the
/// callback, nor bind a reference to them.
///
///
/// # Panic and Exception Tolerance
///
/// The function is non-panicking as long as the callback doesn't panic. The
/// callback can throw an exception.
///
///
///
/// # Example
///
/// ```cpp
///
///   stx::backtrace::trace([](Frame frame, int) {
///    std::cout << "IP=" << frame.ip.as_ref().unwrap() << std::endl;
///    return false;
///  });
///
/// ```
///
///
// all memory passed to the callback is cleared after each call. Hence we only
// use one stack memory for the callback feed-loop.
size_t trace(Callback callback);

/// Installs an handler for the specified signal that prints a backtrace
/// whenever the signal is raised. It can and will only handle `SIGSEGV`,
/// `SIGILL`, and `SIGFPE`. It returns the previous signal handler if
/// successful, else returns the error.
auto handle_signal(int signal) noexcept -> Result<void (*)(int), SignalError>;

};  // namespace backtrace
};  // namespace stx
