/*
 *
 * Copyright (C) 2015 - 2020 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <tntdb/row.h>
#include <tntdb/error.h>

#include <fty_common.h>
#include <fty_common_macros.h>

#include "monitor.h"

// all fields called name
#define MAX_NAME_LENGTH         50

////////////////////////////////////////////////////////////////////////
/////////////////           DEVICE                   ///////////////////
////////////////////////////////////////////////////////////////////////
// devices should have a unique names
db_reply_t
    select_device (tntdb::Connection &conn,
                   const char* device_name)
{
    LOG_START;

    db_reply_t ret = db_reply_new();

    if ( !persist::is_ok_name (device_name) )
    {
        ret.status     = 0;
        log_info ("end: too long device name");
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = TRANSLATE_ME("device name length is not in range [1, MAX_NAME_LENGTH]");
        return ret;
    }

    try{
        // ASSUMPTION: devices have unique names
        tntdb::Statement st = conn.prepare(
            " SELECT"
            "   v.id"
            " FROM"
            "   v_bios_discovered_device v"
            " WHERE "
            "   v.name = :name"
            " LIMIT 1"
        );

        tntdb::Row row = st.set("name", device_name).
                            selectRow();
        log_debug ("1 row was selected");

        row[0].get(ret.item);

        ret.status = 1;

        LOG_END;
        return ret;
    }
    catch (const tntdb::NotFound &e){
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg        = JSONIFY (e.what());
        std::string log_msg = TRANSLATE_ME("end: discovered device was not found with '%s'", ret.msg.c_str ());
        log_info (log_msg.c_str ());
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = JSONIFY(e.what());
        LOG_END_ABNORMAL (e);
        return ret;
    }
}
