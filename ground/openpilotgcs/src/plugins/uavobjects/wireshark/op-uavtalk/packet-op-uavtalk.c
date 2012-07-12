/* packet-op-uavtalk.c
  * Routines for OpenPilot UAVTalk packet dissection
  * Copyright 2012 Stacey Sheldon <stac@solidgoldbomb.org>
  *
  * $Id$
  *
  * Wireshark - Network traffic analyzer
  * By Gerald Combs <gerald@wireshark.org>
  * Copyright 1998 Gerald Combs
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/ptvcursor.h>	/* ptvcursor_* */

#include <glib.h>
#include <string.h>

static guint global_op_uavtalk_port = 9000;

static int proto_op_uavtalk = -1;

static gint ett_op_uavtalk = -1;

static dissector_handle_t data_handle;
static dissector_table_t uavtalk_subdissector_table;

static int hf_op_uavtalk_sync = -1;
static int hf_op_uavtalk_version = -1;
static int hf_op_uavtalk_type = -1;
static int hf_op_uavtalk_len = -1;
static int hf_op_uavtalk_objid = -1;
static int hf_op_uavtalk_crc8 = -1;

#define UAVTALK_SYNC_VAL 0x3C

static const value_string uavtalk_packet_types[]={
  { 0, "TxObj"      },
  { 1, "GetObj"     },
  { 2, "SetObjAckd" },
  { 3, "Ack"        },
  { 4, "Nack"       },
  { 0, NULL         }
};

void proto_reg_handoff_op_uavtalk(void);

#define UAVTALK_HEADER_SIZE 8
#define UAVTALK_TRAILER_SIZE 1
static int dissect_op_uavtalk(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
  gint offset = 0;

  guint8 packet_type = tvb_get_guint8(tvb, 1) & 0x7;
  guint32 objid = tvb_get_letohl(tvb, 4);
  guint32 payload_length = tvb_get_letohs(tvb, 2) - UAVTALK_HEADER_SIZE - UAVTALK_TRAILER_SIZE;
  guint32 reported_length = tvb_reported_length(tvb);

  col_set_str(pinfo->cinfo, COL_PROTOCOL, "UAVTALK");
  /* Clear out stuff in the info column */
  col_clear(pinfo->cinfo, COL_INFO);

  col_append_fstr(pinfo->cinfo, COL_INFO, "%s: 0x%08x", val_to_str_const(packet_type, uavtalk_packet_types, ""), objid);
  if (objid & 0x1) {
    col_append_str(pinfo->cinfo, COL_INFO, "(META)");
  }


  if (tree) { /* we are being asked for details */
    proto_tree *op_uavtalk_tree = NULL;
    ptvcursor_t * cursor;
    proto_item *ti = NULL;

    /* Add a top-level entry to the dissector tree for this protocol */
    ti = proto_tree_add_item(tree, proto_op_uavtalk, tvb, 0, -1, ENC_NA);

    /* Create a subtree to contain the dissection of this protocol */
    op_uavtalk_tree = proto_item_add_subtree(ti, ett_op_uavtalk);

    /* Dissect the packet and populate the subtree */
    cursor = ptvcursor_new(op_uavtalk_tree, tvb, 0);

    /* Populate the fields in this protocol */
    ptvcursor_add(cursor, hf_op_uavtalk_sync, 1, ENC_LITTLE_ENDIAN);
    ptvcursor_add_no_advance(cursor, hf_op_uavtalk_version, 1, ENC_LITTLE_ENDIAN);
    ptvcursor_add(cursor, hf_op_uavtalk_type, 1, ENC_LITTLE_ENDIAN);
    ptvcursor_add(cursor, hf_op_uavtalk_len, 2, ENC_LITTLE_ENDIAN);
    ptvcursor_add(cursor, hf_op_uavtalk_objid, 4, ENC_LITTLE_ENDIAN);

    offset = ptvcursor_current_offset(cursor);

    ptvcursor_free(cursor);

    proto_tree_add_item(op_uavtalk_tree, hf_op_uavtalk_crc8, tvb, reported_length - UAVTALK_TRAILER_SIZE, UAVTALK_TRAILER_SIZE, ENC_LITTLE_ENDIAN);
  } else {
    offset = UAVTALK_HEADER_SIZE;
  }

  {
    tvbuff_t * next_tvb = tvb_new_subset(tvb, offset,
					 reported_length - UAVTALK_HEADER_SIZE - UAVTALK_TRAILER_SIZE,
					 payload_length);

    /* Check if we have an embedded objid to decode */
    if ((packet_type == 0) || (packet_type == 2)) {
      /* Call any registered subdissector for this objid */
      if (!dissector_try_uint(uavtalk_subdissector_table, objid, next_tvb, pinfo, tree)) {
	/* No subdissector registered, use the default data dissector */
	call_dissector(data_handle, next_tvb, pinfo, tree);
      }
    } else {
      /* Render any remaining data as raw bytes */
      call_dissector(data_handle, next_tvb, pinfo, tree);
    }
  }

  return UAVTALK_HEADER_SIZE + UAVTALK_TRAILER_SIZE;
}

void proto_register_op_uavtalk(void)
{
   module_t * op_uavtalk_module;

   static hf_register_info hf[] = {
     { &hf_op_uavtalk_sync,
       { "Sync Byte", "uavtalk.sync", FT_UINT8,
	 BASE_HEX, NULL, 0x0, NULL, HFILL }
     },
     { &hf_op_uavtalk_version,
       { "Version", "uavtalk.ver", FT_UINT8,
	 BASE_DEC, NULL, 0xf8, NULL, HFILL }
     },
     { &hf_op_uavtalk_type,
       { "Type", "uavtalk.type", FT_UINT8,
	 BASE_HEX, VALS(uavtalk_packet_types), 0x07, NULL, HFILL }
     },
     { &hf_op_uavtalk_len,
       { "Length", "uavtalk.len", FT_UINT16,
	 BASE_DEC, NULL, 0x0, NULL, HFILL }
     },
     { &hf_op_uavtalk_objid,
       { "ObjID", "uavtalk.objid", FT_UINT32,
	 BASE_HEX, NULL, 0x0, NULL, HFILL }
     },
     { &hf_op_uavtalk_crc8,
       { "Crc8", "uavtalk.crc8", FT_UINT8,
	 BASE_HEX, NULL, 0x0, NULL, HFILL }
     },
   };

/* Setup protocol subtree array */

   static gint *ett[] = {
         &ett_op_uavtalk
   };

   proto_op_uavtalk = proto_register_protocol("OpenPilot UAVTalk Protocol", 
					      "UAVTALK", 
					      "uavtalk");

   /* Allow subdissectors for each objid to bind for decoding */
   uavtalk_subdissector_table = register_dissector_table("uavtalk.objid", "UAVObject ID", FT_UINT32, BASE_HEX);

   proto_register_subtree_array(ett, array_length(ett));
   proto_register_field_array(proto_op_uavtalk, hf, array_length(hf));

   op_uavtalk_module = prefs_register_protocol(proto_op_uavtalk, proto_reg_handoff_op_uavtalk);

   prefs_register_uint_preference(op_uavtalk_module, "udp.port", "UAVTALK UDP port",
                                  "UAVTALK port (default 9000)", 10, &global_op_uavtalk_port);
}

void proto_reg_handoff_op_uavtalk(void)
{
   static dissector_handle_t op_uavtalk_handle;

   op_uavtalk_handle = new_create_dissector_handle(dissect_op_uavtalk, proto_op_uavtalk);
   dissector_add_handle("udp.port", op_uavtalk_handle);  /* for "decode as" */

   if (global_op_uavtalk_port != 0) {
      dissector_add_uint("udp.port", global_op_uavtalk_port, op_uavtalk_handle);
   }

   /* Lookup the default dissector for raw data */
   data_handle = find_dissector("data");
}
