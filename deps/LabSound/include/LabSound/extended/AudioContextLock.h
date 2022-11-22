// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

//#define DEBUG_LOCKS

#ifndef AUDIO_CONTEXT_LOCK_H
#define AUDIO_CONTEXT_LOCK_H

#include "LabSound/core/AudioContext.h"
#include "LabSound/extended/Logging.h"

#include <iostream>
#include <mutex>

//#define DEBUG_LOCKS

namespace lab
{

class ContextGraphLock
{
    AudioContext * m_context = nullptr;

public:
    ContextGraphLock(AudioContext * context, const std::string & lockSuitor)
    {
#if defined(DEBUG_LOCKS)
        bool reentrant = context->m_graphLocker.size() > 1 && context->m_graphLocker.back() != '~';
        if (reentrant)
        {
            LOG_ERROR("%s cannot acquire an AudioContext ContextGraphLock. Currently held by: %s.", lockSuitor.c_str(), context->m_graphLocker.c_str());
        }
#endif

        if (context)
        {
            context->m_graphLock.lock();
            m_context = context;
            m_context->m_graphLocker = lockSuitor;
        }
    }

    ~ContextGraphLock()
    {
        if (m_context)
        {
            m_context->m_graphLocker += '~';
            m_context->m_graphLock.unlock();
        }
    }

    AudioContext * context() { return m_context; }
};

class ContextRenderLock
{
    AudioContext * m_context = nullptr;

public:
    ContextRenderLock(AudioContext * context, const std::string & lockSuitor)
    {
#if defined(DEBUG_LOCKS)
        bool reentrant = context->m_graphLocker.size() > 1 && context->m_graphLocker.back() != '~';
        if (reentrant)
        {
            LOG_ERROR("%s cannot acquire an AudioContext ContextGraphLock. Currently held by: %s.", lockSuitor.c_str(), context->m_graphLocker.c_str());
        }
#endif

        if (context)
        {
            context->m_renderLock.lock();
            m_context = context;
            m_context->m_renderLocker = lockSuitor;
        }
    }

    ~ContextRenderLock()
    {
        if (m_context)
        {
            m_context->m_renderLocker += '~';
            m_context->m_renderLock.unlock();
        }
    }

    AudioContext * context() { return m_context; }
};

}  // end namespace lab

#endif
