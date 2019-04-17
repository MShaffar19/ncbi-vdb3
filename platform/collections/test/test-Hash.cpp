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

#include <collections/Hash.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

using namespace VDB3;

//#define BENCHMARK

TEST ( Hash, Basic )
{
    const char *str = "Tu estas probando este hoy, no manana";
    size_t size = strlen ( str );

    uint64_t hash = Hash ( str, size );
    ASSERT_NE ( hash, 0 );

    uint64_t hash2 = Hash ( str, size );
    ASSERT_EQ ( hash, hash2 );

    uint64_t hash3 = Hash ( str );
    ASSERT_EQ ( hash, hash3 );

    std::string s = str;
    ASSERT_EQ ( hash, Hash ( s ) );
}

TEST ( Hash, String )
{
    std::string str = "This space for rent";
    uint64_t hash = Hash ( str );
    ASSERT_NE ( hash, 0 );
    ASSERT_EQ ( Hash ( str ), Hash ( str ) );

    str = "";
    std::string str2 = "";
    ASSERT_EQ ( Hash ( str ), Hash ( str ) );
    ASSERT_EQ ( Hash ( str ), Hash ( str2 ) );
}

TEST ( Hash, Integer )
{
    srandom ( static_cast<unsigned int> ( time ( nullptr ) ) );
    auto i = random ();
    uint64_t hash = Hash ( i );
    ASSERT_EQ ( hash, Hash ( i ) );
    i++;
    ASSERT_NE ( hash, Hash ( i ) );
}

TEST ( Hash, Float )
{
    float a = 0.0, b = -0.0;
    ASSERT_EQ ( Hash ( a ), Hash ( b ) );
}

TEST ( Hash, Double )
{
    double a = 0.0, b = -0.0;
    ASSERT_EQ ( Hash ( a ), Hash ( b ) );
    ASSERT_NE ( Hash ( a ), Hash ( b + 1.0 ) );
    ASSERT_NE ( Hash ( a ), Hash ( b + 0.0001 ) );
    ASSERT_NE ( Hash ( a ), Hash ( b - 1.0 ) );
}

TEST ( Hash, Unique )
{
    const char *str = "Tu estas probando este hoy, no manana";
    size_t size = strlen ( str );

    uint64_t hash1 = Hash ( str, size );
    uint64_t hash2 = Hash ( str, size - 1 );
    ASSERT_NE ( hash1, hash2 );
}

/*
TEST ( Hash, Locality )
{
    for ( uint64_t i = 0; i != 8; ++i ) {
        printf ( "hash of %lu is %zx\n", i, Hash ( i ) );
    }
}
*/
TEST ( Hash, Adjacent )
{
    uint64_t hash1, hash2;

    uint64_t val = 0x1234567890ABCDE0;

    hash1 = Hash ( reinterpret_cast<char *> ( &val ), 8 );
    ++val;
    hash2 = Hash ( reinterpret_cast<char *> ( &val ), 8 );
    auto diff = hash2 - hash1;
    ASSERT_LE ( diff, 7 );

    const char *str1 = "nearstring01";
    const char *str2 = "nearstring02";
    size_t size = strlen ( str1 );
    hash1 = Hash ( str1, size );
    hash2 = Hash ( str2, size );
    hash1 &= 0xffffff;
    hash2 &= 0xffffff;
    auto diff2 = hash2 - hash1;
    if ( diff2 > 7 ) {
        fprintf ( stderr, "diff %lx %lx\n", hash1, hash2 );
        ASSERT_LE ( diff2, 7 );
    }
}

TEST ( Hash, StrAdjacent )
{
    std::string str1 = "str02";
    std::string str2 = "str03";
    auto hash1 = Hash ( str1 );
    auto hash2 = Hash ( str2 );
    auto diff = hash2 ^ hash1;
    // printf ( "hash1 is %zx\nhash2 is %zx\n", hash1, hash2 );
    ASSERT_LE ( diff, 7 );
}

TEST ( Hash, Collide )
{
    // We fill a buffer with random bytes, and then increment each byte once
    // and verify no collisions occur for all lengths.
    char buf[37];
    for ( size_t l = 0; l != sizeof ( buf ); l++ )
        buf[l] = static_cast<char> ( random () );

    std::set<uint64_t> set;

    size_t inserts = 0;
    size_t collisions = 0;
    for ( size_t l = 0; l != sizeof ( buf ); l++ )
        for ( size_t j = 0; j != l; j++ )
            for ( size_t k = 0; k != 255; k++ ) {
                buf[j] = static_cast<char> ( buf[j] + 1 );
                uint64_t hash = Hash ( buf, l );
                size_t count = set.count ( hash );
                if ( count != 0 ) {
                    collisions++;
                    fprintf ( stderr,
                        "Collision at byte %lu on hash of len %lu, hash is "
                        "%lx: "
                        "%lu elements in set\n",
                        j, l, hash, set.size () );
                }
                set.insert ( hash );
                inserts++;
            }
    ASSERT_EQ ( inserts, set.size () );
}
#ifdef BENCHMARK

static double stopwatch ( double start = 0.0 )
{
    struct timeval tv_cur = {};
    gettimeofday ( &tv_cur, nullptr );
    double finish = static_cast<double> ( tv_cur.tv_sec )
        + static_cast<double> ( tv_cur.tv_usec ) / 1000000.0;
    // printf("usec=%d\n",tv_cur.tv_usec);
    if ( start == 0.0 ) return finish;
    double elapsed = finish - start;
    if ( elapsed == 0.0 ) elapsed = 1 / 1000000000.0;
    return elapsed;
}


TEST ( Hash, Speed )
{
    char key[16384];
    uint64_t loops = 1000000;

    for ( uint64_t i = 0; i != sizeof ( key ); i++ )
        key[i] = static_cast<char> ( random () );

    uint64_t len = 4;
    uint64_t hash = 0;
    while ( len <= sizeof ( key ) ) {
        double start = stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ ) {
            ++key[0]; // defeat compiler memoization
            hash += Hash ( key, len );
        }

        double elapsed = stopwatch ( start );
        double hps = static_cast<double> ( loops ) / elapsed;
        double mbps = hps * static_cast<double> ( len ) / 1048576.0;
        printf ( "Hash %lu %.1f elapsed (%.1f hash/sec, %.1f Mbytes/sec) %lu\n",
            len, elapsed, hps, mbps, hash & 0xff );

        len *= 2;
    }
}

/*
TEST ( KHash, Speed )
{
    char key[8192];
    uint64_t loops = 1000000;

    for ( uint64_t i = 0; i != sizeof ( key ); i++ )
        key[i] = static_cast<char> ( random () );

    uint64_t len = 4;
    uint64_t hash = 0;
    while ( len  <= sizeof(key) ) {
        double start = stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ ) {
            ++key[0]; // defeat compiler memoization
            hash += KHash ( key, len );
        }

        double elapsed = stopwatch ( start );
        double hps = static_cast<double> ( loops ) / elapsed;
        double mbps = hps * static_cast<double> ( len ) / 1048576.0;
        printf (
            "KHash %lu %.1f elapsed (%.1f hash/sec, %.1f Mbytes/sec) %lu\n",
            len, elapsed, hps, mbps, hash & 0xff );

        len *= 2;
    }
}
TEST ( memset, Speed )
{
    char key[2048576];
    uint64_t loops = 1000000;

    for ( uint64_t i = 0; i != sizeof ( key ); i++ )
        key[i] = static_cast<char> ( random () );

    uint64_t len = 4;
    uint64_t hash = 0;
    while ( len  <= sizeof(key) ) {
        double start = stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ ) {
            ++key[0]; // defeat compiler memoization
            memset(key, 1, len);
        }

        double elapsed = stopwatch ( start );
        double hps = static_cast<double> ( loops ) / elapsed;
        double mbps = hps * static_cast<double> ( len ) / 1048576.0;
        printf (
            "memset %lu %.1f elapsed (%.1f hash/sec, %.1f Mbytes/sec) %lu\n",
            len, elapsed, hps, mbps, hash & 0xff );

        len *= 2;
    }
}
*/
TEST ( Hash, std_hash_Speed )
{
    uint64_t loops = 1000000;
    std::string str = "1234";

    std::size_t hash = 0;
    uint64_t len = 4;
    while ( len <= 8192 ) {
        double start = stopwatch ();
        for ( uint64_t i = 0; i != loops; i++ ) {
            str[0]++; // defeat compiler memoization
            hash += std::hash<std::string> {}( str );
        }

        double elapsed = stopwatch ( start );
        double hps = static_cast<double> ( loops ) / elapsed;
        double mbps = hps * static_cast<double> ( len ) / 1048576.0;
        printf (
            "std::hash %lu %.1f elapsed (%.1f hash/sec, %.1f Mbytes/sec)\n",
            len, elapsed, hps, mbps );

        len *= 2;
        str += str;
    }
}

TEST ( Hash, hamming )
{
    char key[100];
    const uint64_t mask = 0xfff;
    uint64_t hash_collisions[mask + 1];
    uint64_t rhash_collisions[mask + 1];

    for ( uint64_t i = 0; i != mask + 1; i++ ) {
        hash_collisions[i] = 0;
        rhash_collisions[i] = 0;
    }

    for ( uint64_t i = 0; i != 10000000; i++ ) {
        sprintf ( key, "ABCD%lu", i );
        uint64_t hash = Hash ( key, strlen ( key ) );
        hash &= mask;
        hash_collisions[hash] = hash_collisions[hash] + 1;

        hash = static_cast<uint64_t> ( random () );
        hash &= mask;
        rhash_collisions[hash] = rhash_collisions[hash] + 1;
    }

    uint64_t hash_max = 0;
    uint64_t rhash_max = 0;
    for ( uint64_t i = 0; i != mask; i++ ) {
        if ( hash_collisions[i] > hash_max ) hash_max = hash_collisions[i];
        if ( rhash_collisions[i] > rhash_max ) rhash_max = rhash_collisions[i];
    }

    printf ( "rhash longest probe is %lu\n", rhash_max );
}
#endif // BENCHMARK
