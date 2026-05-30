#ifndef MUNGO_INTERNAL_REQUEST_FWD_HPP
#define MUNGO_INTERNAL_REQUEST_FWD_HPP

#include "mungo/attributes.hpp"

namespace mungo {
template <typename Attrs>
class basic_request;

using request = basic_request<AppAttrs<>>;
}  // namespace mungo

#endif  // MUNGO_INTERNAL_REQUEST_FWD_HPP
