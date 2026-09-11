# Some notes for myself on Apple's Metal API

## Resource Lifetime

Apple has their own way of managing memory. When using swift, this is handled
for you via automatic reference counting (ARC) where the compiler inserts
retains and releases into the code for you.

When using the C++ bindings however, this must be done manually :D. For this,
the following methods must be used.

### Methods

- `alloc()` :=
