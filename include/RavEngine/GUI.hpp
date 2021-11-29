#pragma once

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core.h>
#include "Component.hpp"
#include "SpinLock.hpp"
#include "DataStructures.hpp"
#include "Queryable.hpp"
#include "IInputListener.hpp"
#include "Function.hpp"
#include "Ref.hpp"

namespace RavEngine{
class GUIComponent : public RavEngine::Queryable<GUIComponent>, public AutoCTTI{
protected:
	friend class RenderEngine;
	
    struct GUIData : public RavEngine::IInputListener{
        Rml::Context* context = nullptr;
        locked_hashmap<std::string, Rml::ElementDocument*, SpinLock> documents;
        ~GUIData(){
            for (const auto& pair : documents) {
                context->UnloadDocument(pair.second);        //destroy all the documents
            }
            Rml::RemoveContext(context->GetName());
        }
        
        ConcurrentQueue<Function<void(void)>> q_a, q_b;
        std::atomic<decltype(q_a)*> current = &q_a, inactive = &q_b;

        SpinLock mtx;
        
        uint32_t modifier_state = 0;
        
        void AnyActionDown(const int charcode) override;
        
        void AnyActionUp(const int charcode) override;
                
        Rml::Vector2f MousePos;
        
        template<typename T>
        inline void ExclusiveAccess(const T& func){
            mtx.lock();
            func();
            mtx.unlock();
        }
        
        template<typename T>
        constexpr inline void EnqueueUIUpdate(const T& func){
            current.load()->enqueue(func);
        }
    };
    Ref<GUIData> data = std::make_shared<GUIData>();
    
	
public:
    
    inline decltype(data) GetData() const{
        return data;
    }
	
    void MouseMove();
    
	enum class RenderMode{
		Screenspace,
		Worldspace
	} Mode = RenderMode::Screenspace;
	
	/**
	 Construct a GUI document using the current screen size
	 */
	GUIComponent();
    
    GUIComponent(GUIComponent&& other){
        data = std::move(other.data);
    }
	
    GUIComponent(const GUIComponent& other) : GUIComponent(){
        data = other.data;
    }
    
    inline GUIComponent& operator=(const GUIComponent& other){
        if (this != &other){
            data = other.data;
        }
        return *this;
    }
    
	/**
	 Construct a GUI renderer with user-supplied size
	 @param width the width in pixels of the context
	 @param height the height in pixels of the context
	 */
	GUIComponent(int width, int height, float DPI = 1);
	
	inline void SetDPIScale(float scale){
		data->context->SetDensityIndependentPixelRatio(scale);
	}
	
	/**
	 Load a document from disk with a name
	 @param name the name of the RML file
	 @throws if this document has already been loaded into this context
	 */
	Rml::ElementDocument* AddDocument(const std::string& name);
	
	/**
	 Unload a document from disk with a name
	 @param name the name of the RML file
	 @throws if this document is not loaded
	 */
	void RemoveDocument(const std::string& name);
	
	/**
	 @returns true if a document with the passed name is currently loaded.
	 @param name the name of the RML file
	 */
	bool IsDocumentLoaded(const std::string& name) const;
	
	/**
	 Get a pointer to a document, for performing queries or bindings.
	 @param name the name of the document.
	 @return a pointer to the RML ElementDocument. Use of this pointer is not safe outside of ExclusiveAccess.
	 @throws if the document is not loaded.
	 */
	Rml::ElementDocument* GetDocument(const std::string& name) const;
	
	/**
	 Change the size of the context.
	 @param width the new width of the context
	 @param height the new height of the context
	 */
	void SetDimensions(uint32_t width, uint32_t height);
	
	/** Bind to your InputManager's mouse X axis mapping
	 * @param normalized_pos the position of the mouse, in [0,1)
	 */
	void MouseX(float normalized_pos);
	
	/** Bind to your InputManager's mouse Y axis mapping
	 * @param normalized_pos the position of the mouse, in [0,1)
	 */
	void MouseY(float normalized_pos);
	
	/** Bind to your InputManager's scroll Y axis mapping
	 * @param normalized_pos the position of the mouse, in [0,1)
	 */
	void ScrollY(float amt);

	/**
	Execute code on this element with exclusive thread-safe access. For internal use only.
	@param func a lambda to execute. 
	*/
    template<typename T>
    constexpr inline void ExclusiveAccess(const T& func) {
        data->ExclusiveAccess(func);
	}

    /**
    Schedule a UI update. Use this call whenever you are making changes to the UI, to avoid threading issues
    @param func a capturing lambda to execute. Be sure to capture by value! The passed function is executed on a different thread at a different time than the caller.
    */
    template<typename T>
    constexpr inline void EnqueueUIUpdate(const T& func){
        data->EnqueueUIUpdate(func);
    }
	
	/**
	* Load a font globally
	*/
	static bool LoadFont(const std::string& filename);
	
	/**
	* Set the RML debugger to focus on this context
	*/
	void Debug();
	
	virtual ~GUIComponent();
	
	/**
	* Recalculate based on changes made (internal use only)
	*/
	bool Update();
	
	/**
	* Issue draw calls to render this element (internal use only)
	*/
	bool Render();
};
}

