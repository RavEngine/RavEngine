#pragma once
#include <type_traits>

namespace RavEngine {
    struct PhysicsSolver;
    struct World;

    struct WorldDataProvider {
        World* world = nullptr;
    };

    struct PhysicsSolverProvider {
        PhysicsSolver* physicsSolver = nullptr;
    };

    // used for is_convertible checks
    struct ValidatorProviderBase {};

    template<typename ... A>
    struct ValidatorProvider : public ValidatorProviderBase {
        friend class World;
        using ValidatorType = Validator<A...>;

        ValidatorType validator;

        // no copy
        ValidatorProvider(const ValidatorProvider&) = delete;
        void operator=(ValidatorProvider const&) = delete;
    protected:
        // private move, private construct
        ValidatorProvider(ValidatorProvider&&) = default;
        ValidatorProvider() {}
    };

    // if the System does not need Engine data, 
    // the FuncMode will default to this type
    struct DataProviderNone {};

    template <typename T>
    concept IsEngineDataProvider =
        (std::is_convertible_v<T, WorldDataProvider> || std::is_convertible_v<T, ValidatorProviderBase> || std::is_convertible_v<T, PhysicsSolverProvider>)
        && not (std::is_convertible_v<T, DataProviderNone>);
}
