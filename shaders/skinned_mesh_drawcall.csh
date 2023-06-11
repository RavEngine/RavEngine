layout(push_constant) uniform UniformBufferObject{
    uint indexBufferOffset;
    uint vertexBufferOffset;
    uint nIndicesInThisMesh;
    uint nVerticesInThisMesh;
    uint nTotalObjects;
    uint drawCallBufferOffset;
    uint baseInstanceOffset;
} ubo;

struct IndirectCommand {
	uint indexCount;
	uint instanceCount;
	uint indexStart;
	uint baseVertex;
	uint baseInstance;
};

layout(std430, binding = 0) buffer indirectDrawBuffer
{
	IndirectCommand commands[];
};

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;	//x = object #
void main(){
    const uint objectID = gl_GlobalInvocationID.x;
    // bail
    if (objectID >= ubo.nTotalObjects){
        return;
    }

    commands[ubo.drawCallBufferOffset + objectID] = IndirectCommand(
        ubo.nIndicesInThisMesh, // indexCount
        0,                      // instanceCount (we may end up with many zero-instance draws but that is OK for now)
        ubo.indexBufferOffset + ubo.nIndicesInThisMesh * objectID,  // indexStart,
        ubo.vertexBufferOffset + ubo.nVerticesInThisMesh * objectID,    // baseVertex,
        ubo.baseInstanceOffset + objectID                                       // baseInstance
    );
}