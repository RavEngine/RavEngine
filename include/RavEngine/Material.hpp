
#include "SharedObject.hpp"
#include <bgfx/bgfx.h>

class Material : public SharedObject {
public:
	Material();

	void Submit() {
		bgfx::submit(0, program);
	}

protected:
	bgfx::ProgramHandle program;
};