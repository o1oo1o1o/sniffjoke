/*
 *   SniffJoke is a software able to confuse the Internet traffic analysis,
 *   developed with the aim to improve digital privacy in communications and
 *   to show and test some securiy weakness in traffic analysis software.
 *   
 *   Copyright (C) 2010 vecna <vecna@delirandom.net>
 *                      evilaliv3 <giovanni.pellerano@evilaliv3.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SJ_PLUGIN_H
#define SJ_PLUGIN_H

#include "Utils.h"
#include "Packet.h"

/* 
 *
 * Following this howto: http://www.faqs.org/docs/Linux-mini/C++-dlopen.html
 * we understood how to do a plugin and load it.
 * HackPacket classes are implemented as external modules and the programmer
 * shoulds implement condition and applyPlugin, constructor and distructor methods.
 *
 * At the end of every plugin code, it's is required to export two "C" symbols,
 * pointing to the constructor and the destructor method.
 *
 */

class cacheRecord
{
public:
    time_t access_timestamp;

    const Packet cached_packet;
    vector<unsigned char>cached_data;

    cacheRecord(const Packet& pkt) :
    access_timestamp(sj_clock),
    cached_packet(pkt)
    {
    };

    cacheRecord(const Packet& pkt, const unsigned char* data, size_t data_size) :
    access_timestamp(sj_clock),
    cached_packet(pkt),
    cached_data(data, data + data_size)
    {
    };
};

class Plugin
{
public:

    uint8_t supportedScrambles; /* supported by the location, derived
                                  from plugin_enabler.conf.$location */
    const char * const pluginName; /* plugin name as const string */
    const uint16_t pluginFrequency; /* plugin frequency, using the value  */
    bool removeOrigPkt; /* boolean to be set true if the plugin
                           needs to remove the original packet */

    vector<Packet *> pktVector; /* std vector of Packet* used for created packets */
    vector<cacheRecord *> pluginCache;
    time_t pluginCacheTimeout;

    judge_t pktRandomDamage(uint8_t scrambles)
    {
        if (ISSET_TTL(scrambles) && RANDOMPERCENT(75))
            return PRESCRIPTION;
        if (ISSET_MALFORMED(scrambles) && RANDOMPERCENT(80))
            return MALFORMED;
        return GUILTY;
    }

    Plugin(const char* pluginName, uint16_t pluginFrequency) :
    pluginName(pluginName),
    pluginFrequency(pluginFrequency),
    removeOrigPkt(false),
    pluginCacheTimeout(PLUGINCACHE_EXPIRYTIME)
    {
    };

    virtual void applyPlugin(const Packet &, uint8_t availableScrambles) = 0;

    virtual bool condition(const Packet &, uint8_t availableScrambles)
    {
        return true;
    };

    virtual bool initializePlugin(uint8_t configuredScramble)
    {
        return true;
    };

    virtual void mangleIncoming(Packet &pkt)
    {
    };

    virtual void reset(void)
    {
        removeOrigPkt = false;
        pktVector.clear();
    }

    vector<cacheRecord *>::iterator cacheCheck(bool(*filter)(const cacheRecord &, const Packet &), const Packet &);
    vector<cacheRecord *>::iterator cacheCreate(const Packet &);
    vector<cacheRecord *>::iterator cacheCreate(const Packet &, const unsigned char* data, size_t data_size);
    void cacheDelete(vector<struct cacheRecord *>::iterator it);

    void upgradeChainFlag(Packet *);
};

#endif /* SJ_PLUGIN_H */