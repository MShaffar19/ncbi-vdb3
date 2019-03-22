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
#pragma once

#include <memory/MemoryBlockItf.hpp>

namespace VDB3
{

/**
 *  A raw memory block with exactly one owner.
 */
class UniqueRawMemoryBlock : public MemoryBlockItf
{
public:
    /**
     * Constructor
     * @param mgr instance of a memory manager that allocated this block. Will be used for deallocation.
     * @param size size of the block
    */
    UniqueRawMemoryBlock ( MemoryManagerItf & mgr, size_t size );

    /**
     * Move constructor.
     * @param that source
     */
    UniqueRawMemoryBlock ( UniqueRawMemoryBlock && that );

    virtual ~UniqueRawMemoryBlock();

public: // inherited from MemoryBlockItf

    virtual bytes_t size () const;

public:

    /**
     * Change the size of the block
     * @param new_size new size of the block
     * @exception TODO: ??? if reallocaiton failed
     */
    void resize ( bytes_t new_size );

    /**
     * Read/write access to the bytes
     * @return underlying bytes
     */
    byte_t * data() noexcept { return m_ptr . get (); }

    /**
     * Read access to the bytes
     * @return underlying bytes
     */
    const byte_t * data() const noexcept { return m_ptr . get (); }

    /**
     * Fill out the block with specified value
     * @param filler vallue to fill the block with
     */
    void fill ( byte_t filler );

    /**
     * Create a copy of the block
     * @return a copy of the block
     * @exception TODO: ??? if failed
     */
    UniqueRawMemoryBlock clone () const;

protected:
    /**
     * Uniquely owned pointer to the sequence bytes
    */
    typedef std :: unique_ptr < byte_t, Deleter < byte_t > > PtrType;

    /**
     * Read/write access to the pointer
     * @return the pointer
    */
    PtrType & getPtr() { return m_ptr; }

    /**
     * Read access to the pointer
     * @return the pointer
    */
    const PtrType & getPtr() const { return m_ptr; }

private:
    /**
     * Assignment - deleted
     * @param that source
     * @return *this
    */
    UniqueRawMemoryBlock & operator = ( const UniqueRawMemoryBlock & that );

    size_t m_size;  ///< size of the block, in bytes
    PtrType m_ptr;  ///< pointer to the underlying bytes
};

} // namespace VDB3

