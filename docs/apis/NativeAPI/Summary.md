# Script layer native interface (FFI)

!!! warning
    NativeAPI has broken, don't use it in production environment

By providing FFI (Foreign Function Interface) related interfaces in the script layer, it helps to simplify the development process of small repair plugins, and obtain a more stable ABI through the script engine, which is convenient for sharing plugins across multiple versions

> This kind of API is only suitable for workflows that are not performance sensitive, and the caller has a thorough and accurate understanding of the native language, otherwise it is very easy to cause unexpected situations.
> If you need high-performance low-level interfaces, please use native languages.
> Since the FFI interface specification is still under design, the interface and related information may change at any time without notice.
