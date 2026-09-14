#pragma once

namespace MTL {
class AccelerationStructure;
class AccelerationStructureDescriptor;
class Architecture;
class ArgumentDescriptor;
class ArgumentEncoder;
class BinaryArchive;
class BinaryArchiveDescriptor;
class Buffer;
class BufferBinding;
class CommandBuffer;
class CommandQueue;
class CommandQueueDescriptor;
class CompileOptions;
class ComputePipelineDescriptor;
class ComputePipelineReflection;
class ComputePipelineState;
class CounterSampleBuffer;
class CounterSampleBufferDescriptor;
class DepthStencilDescriptor;
class DepthStencilState;
class Device;
class DynamicLibrary;
class Event;
class Fence;
class Function;
class FunctionConstantValues;
class FunctionHandle;
class Heap;
class HeapDescriptor;
class IOCommandQueue;
class IOCommandQueueDescriptor;
class IOFileHandle;
class IndirectCommandBuffer;
class IndirectCommandBufferDescriptor;
class Library;
class LogState;
class LogStateDescriptor;
class MeshRenderPipelineDescriptor;
class RasterizationRateMap;
class RasterizationRateMapDescriptor;
struct Region;
class RenderCommandEncoder;
class RenderPipelineDescriptor;
class RenderPipelineReflection;
class RenderPipelineState;
class ResidencySet;
class ResidencySetDescriptor;
class ResourceViewPoolDescriptor;
struct SamplePosition;
class SamplerDescriptor;
class SamplerState;
class SharedEvent;
class SharedEventHandle;
class SharedTextureHandle;
class StitchedLibraryDescriptor;
class Tensor;
class TensorDescriptor;
class Texture;
class TextureDescriptor;
class TextureViewPool;
class TileRenderPipelineDescriptor;
}; // namespace MTL

namespace MTL4 {
class Archive;
class ArgumentTable;
class ArgumentTableDescriptor;
class BinaryFunction;
class CommandAllocator;
class CommandAllocatorDescriptor;
class CommandBuffer;
class CommandQueue;
class CommandQueueDescriptor;
class Compiler;
class CompilerDescriptor;
class CounterHeap;
class CounterHeapDescriptor;
class PipelineDataSetSerializer;
class PipelineDataSetSerializerDescriptor;
class RenderCommandEncoder;

} // namespace MTL4

namespace CA {
class MetalLayer;
class MetalDrawable;
} // namespace CA

namespace NS {
class AutoreleasePool;
class Error;

} // namespace NS
