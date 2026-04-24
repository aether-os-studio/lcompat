#pragma once

#define TRACE_HEADER_MULTI_READ

#define TP_PROTO(...) (__VA_ARGS__)
#define TP_ARGS(...)
#define TP_STRUCT__entry(...)
#define TP_fast_assign(...)
#define TP_printk(...)
#define __field(type, item)
#define __array(type, item, len)

#define DECLARE_EVENT_CLASS(name, proto, args, struct_entry, assign, print)    \
    static inline void trace_##name proto {}

#define DEFINE_EVENT(template, name, proto, args)                              \
    static inline void trace_##name proto {}

#define TRACE_EVENT(name, proto, args, struct_entry, assign, print)            \
    static inline void trace_##name proto {}

#define EXPORT_TRACEPOINT_SYMBOL_GPL(name)
