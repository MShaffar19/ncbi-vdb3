/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include <memory/QuotaMemoryMgr.hpp>

#include <cassert>

#include <memory/PrimordialMemoryMgr.hpp>

using namespace VDB3;

QuotaMemoryMgr :: QuotaMemoryMgr( TrackingMemoryMgr base_mgr, bytes_t quota )
:   TrackingBypassMemoryManager ( base_mgr ),
    m_quota ( quota ),
    m_used ( 0 )
{
}

QuotaMemoryMgr :: QuotaMemoryMgr( bytes_t quota )
:   m_quota ( quota ),
    m_used ( 0 )
{
}

QuotaMemoryMgr :: ~QuotaMemoryMgr()
{
}

QuotaMemoryMgr :: pointer
QuotaMemoryMgr :: allocate ( size_type bytes )
{
    if ( m_used + bytes > m_quota )
    {
        handle_quota_update ( m_used + bytes - m_quota );
        // will not return if fails
    }
    pointer ret = baseMgr () -> allocate ( bytes );
    if ( ret != nullptr ) // nullptr can happen if bytes == 0
    {
        m_used += bytes;
    }
    return ret;
}

QuotaMemoryMgr :: pointer
QuotaMemoryMgr :: reallocate ( pointer old_ptr, size_type new_size )
{
    if ( old_ptr == nullptr )
    {
        return allocate ( new_size );
    }

    size_type old_size = getBlockSize ( old_ptr );
    if ( new_size > old_size )
    {
        size_type new_total = m_used + new_size - old_size;
        if ( new_total > m_quota )
        {
            handle_quota_update ( new_total - m_quota );
            // will not return if fails
        }
    }

    pointer new_ptr = baseMgr () -> reallocate ( old_ptr, new_size );

    // discount the old block size
    m_used -= old_size;

    if ( new_ptr != nullptr ) // can be if new_size == 0
    {
        m_used += new_size;
    }

    return new_ptr;
}

void
QuotaMemoryMgr :: deallocate ( pointer ptr, size_type bytes ) noexcept
{
    if ( ptr == nullptr )
    {
        return;
    }

    baseMgr () -> deallocate ( ptr, bytes );

    m_used -= bytes;
}

void
QuotaMemoryMgr :: handle_quota_update( bytes_t min_extension )
{
    if ( ! update_quota ( min_extension ) )
    {
        throw std :: bad_alloc (); //TODO: use a VDB3 exception when implemented
    }
}

