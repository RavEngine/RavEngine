
#include "SharedObject.hpp"

namespace RavEngine {
	class Material : public SharedObject {
	public:
		Material();

		void Submit() {
			//bgfx::submit(0, program);
		}

	protected:
	};
}
