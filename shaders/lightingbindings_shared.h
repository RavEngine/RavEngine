
layout(location = 0) out vec4 outcolor;

layout(binding = 0) uniform sampler g_sampler;
layout(binding = 1) uniform sampler shadowSampler;
layout(binding = 2) uniform texture2D t_albedo;
layout(binding = 3) uniform texture2D t_normal;
layout(binding = 4) uniform texture2D t_depth;
layout(binding = 5) uniform texture2D t_depthshadow;
layout(binding = 6) uniform texture2D t_roughnessSpecularMetallicAO;