# TODO

- [x] Copy-On-Write support
- [ ] Pinned Memory Support (Need CUDA support)
- [ ] Memory Pooling and Caching
- [ ] Fix reference counting

Design Thougts

- I think it'll be better to have reference counting within DataPtr, so that whenever some storage instant points to a DataPtr, counts get incremented (instead of manually handling the increment/decrement of counts)
