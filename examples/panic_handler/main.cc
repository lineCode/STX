#include <iostream>

#include "stx/panic.h"

int main() { stx::panic("crash and burn!"); }

void stx::panic_handler(std::string_view info,
                        stx::ReportPayload const& payload,
                        stx::SourceLocation source_location) noexcept {
  std::cerr << "Error Occurred"
            << "\nInfo: " << info  //
            << "\nPayload:" << payload.data() << std::endl;
}