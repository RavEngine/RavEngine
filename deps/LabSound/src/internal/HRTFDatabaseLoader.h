// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef HRTFDatabaseLoader_h
#define HRTFDatabaseLoader_h

#include "internal/HRTFDatabase.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace lab
{

// HRTFDatabaseLoader will asynchronously load the default HRTFDatabase in a new thread.

class HRTFDatabaseLoader
{

public:
    // Both constructor and destructor must be called from the main thread.
    // It's expected that the singletons will be accessed instead.
    // @CBB the guts of the loader should be a private singleton, so that the loader can be constructed without a factory
    explicit HRTFDatabaseLoader(float sampleRate, const std::string & searchPath);

    // Lazily creates the singleton HRTFDatabaseLoader (if not already created) and starts loading asynchronously (when created the first time).
    // Returns the singleton HRTFDatabaseLoader.
    // Must be called from the main thread.
    static std::shared_ptr<HRTFDatabaseLoader> MakeHRTFLoaderSingleton(float sampleRate, const std::string & searchPath);

    // Returns the singleton HRTFDatabaseLoader.
    static std::shared_ptr<HRTFDatabaseLoader> loader() { return s_loader; }

    // Both constructor and destructor must be called from the main thread.
    ~HRTFDatabaseLoader();

    // Returns true once the default database has been completely loaded.
    bool isLoaded() const;

    // waitForLoaderThreadCompletion() may be called more than once and is thread-safe.
    void waitForLoaderThreadCompletion();

    HRTFDatabase * database() { return m_hrtfDatabase.get(); }

    float databaseSampleRate() const { return m_databaseSampleRate; }

    // Called in asynchronous loading thread.
    void load();

    // defaultHRTFDatabase() gives access to the loaded database.
    // This can be called from any thread, but it is the callers responsibilty to call this while the context (and thus HRTFDatabaseLoader)
    // is still alive.  Otherwise this will return 0.
    static HRTFDatabase * defaultHRTFDatabase();

private:
    static void databaseLoaderEntry(HRTFDatabaseLoader * threadData);

    // If it hasn't already been loaded, creates a new thread and initiates asynchronous loading of the default database.
    // This must be called from the main thread.
    void loadAsynchronously();

    static std::shared_ptr<HRTFDatabaseLoader> s_loader;  // singleton
    std::unique_ptr<HRTFDatabase> m_hrtfDatabase;

    // Holding a m_threadLock is required when accessing m_databaseLoaderThread.
    std::mutex m_threadLock;
    std::thread m_databaseLoaderThread;
    std::condition_variable m_loadingCondition;
    bool m_loading;

    float m_databaseSampleRate;

    std::string searchPath;
};

}  // namespace lab

#endif  // HRTFDatabaseLoader_h
