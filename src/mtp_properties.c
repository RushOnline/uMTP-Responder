/*
 * uMTP Responder
 * Copyright (c) 2018 - 2019 Viveris Technologies
 *
 * uMTP Responder is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * uMTP Responder is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 3 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uMTP Responder; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file   mtp_properties.c
 * @brief  MTP properties datasets helpers
 * @author Jean-Fran�ois DEL NERO <Jean-Francois.DELNERO@viveris.fr>
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/statvfs.h>

#include <time.h>

#include "logs_out.h"

#include "mtp_helpers.h"

#include "fs_handles_db.h"

#include "mtp.h"
#include "mtp_datasets.h"
#include "mtp_properties.h"

#include "usb_gadget_fct.h"

#include "mtp_constant.h"
#include "mtp_constant_strings.h"

#include "mtp_support_def.h"

profile_property properties[]=
{   // prop_code                       data_type         getset    default value          group code
	{MTP_PROPERTY_STORAGE_ID,          MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000001 , 0x00 },
	{MTP_PROPERTY_OBJECT_FORMAT,       MTP_TYPE_UINT16,    0x00,   0x3000               , 0x000000001 , 0x00 },
	{MTP_PROPERTY_ASSOCIATION_TYPE,    MTP_TYPE_UINT16,    0x00,   0x0001               , 0x000000001 , 0x00 },
	{MTP_PROPERTY_ASSOCIATION_DESC,    MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000001 , 0x00 },
	{MTP_PROPERTY_PROTECTION_STATUS,   MTP_TYPE_UINT16,    0x00,   0x0000               , 0x000000001 , 0x00 },
	{MTP_PROPERTY_HIDDEN,              MTP_TYPE_UINT16,    0x00,   0x0000               , 0x000000001 , 0x00 },
	{MTP_PROPERTY_PROTECTION_STATUS,   MTP_TYPE_UINT16,    0x00,   0x0000               , 0x000000001 , 0x00 },
	{MTP_PROPERTY_OBJECT_SIZE,         MTP_TYPE_UINT64,    0x00,   0x0000000000000000   , 0x000000001 , 0x00 },
	{MTP_PROPERTY_OBJECT_FILE_NAME,    MTP_TYPE_STR,       0x01,   0x00                 , 0x000000001 , 0x00 },
	{MTP_PROPERTY_DATE_CREATED,        MTP_TYPE_STR,       0x00,   0x00                 , 0x000000001 , 0x00 },
	{MTP_PROPERTY_DATE_MODIFIED,       MTP_TYPE_STR,       0x00,   0x00                 , 0x000000001 , 0x00 },
	{MTP_PROPERTY_PARENT_OBJECT,       MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000001 , 0x00 },

	{0xFFFF,                           MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000000 , 0x00 }
};

profile_property dev_properties[]=
{   // prop_code                                           data_type         getset    default value          group code
	//{MTP_DEVICE_PROPERTY_SYNCHRONIZATION_PARTNER,          MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000000 , 0x00 },
	//{MTP_DEVICE_PROPERTY_IMAGE_SIZE,                       MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000000 , 0x00 },
	{MTP_DEVICE_PROPERTY_BATTERY_LEVEL,                    MTP_TYPE_UINT16,    0x00,   0x00000000           , 0x000000000 , 0x00 },
	{MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,             MTP_TYPE_STR,       0x00,   0x00000000           , 0x000000000 , 0x00 },

	{0xFFFF,                                               MTP_TYPE_UINT32,    0x00,   0x00000000           , 0x000000000 , 0x00 }
};

int build_properties_dataset(mtp_ctx * ctx,void * buffer, int maxsize,uint32_t property_id,uint32_t format_id)
{
	int ofs,i;

	ofs = 0;

	PRINT_DEBUG("build_properties_dataset : 0x%.4X (%s) (Format : 0x%.4X - %s )", property_id, mtp_get_property_string(property_id), format_id, mtp_get_format_string(format_id));

	i = 0;
	while(properties[i].prop_code != 0xFFFF && properties[i].prop_code != property_id)
	{
		i++;
	}

	if( properties[i].prop_code == property_id )
	{
		poke(buffer, &ofs, 2, properties[i].prop_code);            // PropertyCode
		poke(buffer, &ofs, 2, properties[i].data_type);            // DataType
		poke(buffer, &ofs, 1, properties[i].getset);               // Get / Set

		switch(properties[i].data_type)
		{
			case MTP_TYPE_STR:
			case MTP_TYPE_UINT8:
				poke(buffer, &ofs, 1, properties[i].default_value);                         // DefaultValue
			break;
			case MTP_TYPE_UINT16:
				poke(buffer, &ofs, 2, properties[i].default_value);                         // DefaultValue
			break;
			case MTP_TYPE_UINT32:
				poke(buffer, &ofs, 4, properties[i].default_value);                         // DefaultValue
			break;
			case MTP_TYPE_UINT64:
				poke(buffer, &ofs, 4, properties[i].default_value & 0xFFFFFFFF);            // DefaultValue
				poke(buffer, &ofs, 4, properties[i].default_value >> 32);
			break;
			default:
				PRINT_ERROR("build_properties_dataset : Unsupported data type : 0x%.4X", dev_properties[i].data_type );			
			break;
		}

		poke(buffer, &ofs, 4, properties[i].group_code);           // Group code
		poke(buffer, &ofs, 1, properties[i].form_flag);            // Form flag
	}

	return ofs;
}

int build_device_properties_dataset(mtp_ctx * ctx,void * buffer, int maxsize,uint32_t property_id)
{
	int ofs,i;

	ofs = 0;

	PRINT_DEBUG("build_device_properties_dataset : 0x%.4X (%s)", property_id, mtp_get_property_string(property_id));

	i = 0;
	while(dev_properties[i].prop_code != 0xFFFF && dev_properties[i].prop_code != property_id)
	{
		i++;
	}

	if( dev_properties[i].prop_code == property_id )
	{
		poke(buffer, &ofs, 2, dev_properties[i].prop_code);            // PropertyCode
		poke(buffer, &ofs, 2, dev_properties[i].data_type);            // DataType
		poke(buffer, &ofs, 1, dev_properties[i].getset);               // Get / Set

		switch(dev_properties[i].data_type)
		{
			case MTP_TYPE_STR:
			case MTP_TYPE_UINT8:
				poke(buffer, &ofs, 1, dev_properties[i].default_value);
				poke(buffer, &ofs, 1, dev_properties[i].default_value);
			break;
			case MTP_TYPE_UINT16:
				poke(buffer, &ofs, 2, dev_properties[i].default_value);
				poke(buffer, &ofs, 2, dev_properties[i].default_value);
			break;
			case MTP_TYPE_UINT32:
				poke(buffer, &ofs, 4, dev_properties[i].default_value);
				poke(buffer, &ofs, 4, dev_properties[i].default_value);
			break;
			case MTP_TYPE_UINT64:
				poke(buffer, &ofs, 4, dev_properties[i].default_value & 0xFFFFFFFF);
				poke(buffer, &ofs, 4, dev_properties[i].default_value >> 32);
				poke(buffer, &ofs, 4, dev_properties[i].default_value & 0xFFFFFFFF);
				poke(buffer, &ofs, 4, dev_properties[i].default_value >> 32);
			break;
			default:
				PRINT_ERROR("build_device_properties_dataset : Unsupported data type : 0x%.4X", dev_properties[i].data_type );
			break;
		}

		poke(buffer, &ofs, 4, dev_properties[i].group_code);           // Group code
		poke(buffer, &ofs, 1, dev_properties[i].form_flag);            // Form flag
	}

	return ofs;
}


int build_properties_supported_dataset(mtp_ctx * ctx,void * buffer, int maxsize,uint32_t format_id)
{
	int ofs,i;
	int nb_supported_prop;

	PRINT_DEBUG("build_properties_supported_dataset : (Format : 0x%.4X - %s )", format_id, mtp_get_format_string(format_id));

	nb_supported_prop = 0;

	while( properties[nb_supported_prop].prop_code != 0xFFFF )
		nb_supported_prop++;

	ofs = 0;

	poke(buffer, &ofs, 4, nb_supported_prop);

	i = 0;
	while( properties[i].prop_code != 0xFFFF )
	{
		poke(buffer, &ofs, 2, properties[i].prop_code);
		i++;
	}

	return ofs;
}

int setObjectPropValue(mtp_ctx * ctx,MTP_PACKET_HEADER * mtp_packet_hdr, uint32_t handle,uint32_t prop_code)
{
	fs_entry * entry;
	char * path;
	char * path2;
	char tmpstr[256+1];
	unsigned int stringlen;

	PRINT_DEBUG("setObjectPropValue : (Handle : 0x%.8X - Prop code : 0x%.4X )", handle, prop_code);

	if( handle != 0xFFFFFFFF )
	{
		switch( prop_code )
		{
			case MTP_PROPERTY_OBJECT_FILE_NAME:
				entry = get_entry_by_handle(ctx->fs_db, handle);
				if(!entry)
					return MTP_RESPONSE_INVALID_OBJECT_HANDLE;

				path = build_full_path(ctx->fs_db, mtp_get_storage_root(ctx, entry->storage_id), entry);
				if(!path)
					return MTP_RESPONSE_GENERAL_ERROR;

				memset(tmpstr,0,sizeof(tmpstr));

				stringlen = peek(mtp_packet_hdr, sizeof(MTP_PACKET_HEADER), 1);

				if( stringlen > sizeof(tmpstr))
					stringlen = sizeof(tmpstr);

				unicode2charstring(tmpstr, (uint16_t *) ((char*)(mtp_packet_hdr) + sizeof(MTP_PACKET_HEADER) + 1), stringlen);

				if( entry->name )
				{
					free(entry->name);
					entry->name = NULL;
				}

				entry->name = malloc(strlen(tmpstr)+1);
				if( entry->name )
				{
					strcpy(entry->name,tmpstr);
				}

				path2 = build_full_path(ctx->fs_db, mtp_get_storage_root(ctx, entry->storage_id), entry);
				if(!path2)
				{
					free(path);
					return MTP_RESPONSE_GENERAL_ERROR;
				}

				if(rename(path, path2))
				{
					PRINT_ERROR("setObjectPropValue : Can't rename %s to %s", path, path2);

					free(path);
					free(path2);
					return MTP_RESPONSE_GENERAL_ERROR;
				}

				free(path);
				free(path2);
				return MTP_RESPONSE_OK;
			break;

			default:
				return MTP_RESPONSE_INVALID_OBJECT_PROP_CODE;
			break;
		}
	}
	else
	{
		return MTP_RESPONSE_INVALID_OBJECT_HANDLE;
	}
}

int build_ObjectPropValue_dataset(mtp_ctx * ctx,void * buffer, int maxsize,uint32_t handle,uint32_t prop_code)
{
	int ofs;
	fs_entry * entry;
	char timestr[32];

	ofs = 0;

	PRINT_DEBUG("build_ObjectPropValue_dataset : Handle 0x%.8X - Property 0x%.4X (%s)", handle, prop_code, mtp_get_property_string(prop_code));

	entry = get_entry_by_handle(ctx->fs_db, handle);
	if( entry )
	{
		switch(prop_code)
		{
			case MTP_PROPERTY_OBJECT_FORMAT:
				if(entry->flags & ENTRY_IS_DIR)
					poke(buffer, &ofs, 2, MTP_FORMAT_ASSOCIATION);                          // ObjectFormat Code
				else
					poke(buffer, &ofs, 2, MTP_FORMAT_UNDEFINED);                            // ObjectFormat Code
			break;

			case MTP_PROPERTY_OBJECT_SIZE:
				poke(buffer, &ofs, 4, entry->size);
				poke(buffer, &ofs, 4, 0x00000000);    // TODO : Proper 64bits support !
			break;

			case MTP_PROPERTY_OBJECT_FILE_NAME:
				poke_string(buffer, &ofs,entry->name);                                      // Filename
			break;

			case MTP_PROPERTY_STORAGE_ID:
				poke(buffer, &ofs, 4, entry->storage_id);
			break;

			case MTP_PROPERTY_PARENT_OBJECT:
					poke(buffer, &ofs, 4, entry->parent);
			break;

			case MTP_PROPERTY_HIDDEN:
					poke(buffer, &ofs, 2, 0x0000);
			break;

			case MTP_PROPERTY_SYSTEM_OBJECT:
					poke(buffer, &ofs, 2, 0x0000);
			break;

			case MTP_PROPERTY_PROTECTION_STATUS:
				poke(buffer, &ofs, 2, 0x0000);
			break;

			case MTP_PROPERTY_ASSOCIATION_TYPE:
				if(entry->flags & ENTRY_IS_DIR)
						poke(buffer, &ofs, 2, 0x0001);                          // ObjectFormat Code
				else
						poke(buffer, &ofs, 2, 0x0000);                          // ObjectFormat Code
			break;

			case MTP_PROPERTY_ASSOCIATION_DESC:
				poke(buffer, &ofs, 4, 0x00000000);
			break;


			case MTP_PROPERTY_DATE_CREATED:
			case MTP_PROPERTY_DATE_MODIFIED:
				snprintf(timestr,sizeof(timestr),"%.4d%.2d%.2dT%.2d%.2d%.2d",1900 + 110, 1, 2, 10, 11,12);
				poke_string(buffer, &ofs,timestr);
			break;

			default:
				PRINT_ERROR("build_ObjectPropValue_dataset : Unsupported property : 0x%.4X (%s)", prop_code, mtp_get_property_string(prop_code));
			break;
		}
	}

	return ofs;
}

int objectproplist_element(mtp_ctx * ctx, void * buffer, int * ofs, uint16_t prop_code, uint32_t handle, void * data)
{
	int i;

	i = 0;
	while(properties[i].prop_code != 0xFFFF && properties[i].prop_code != prop_code)
	{
		i++;
	}

	if( properties[i].prop_code == prop_code )
	{
		poke(buffer, ofs, 4, handle);
		poke(buffer, ofs, 2, properties[i].prop_code);
		poke(buffer, ofs, 2, properties[i].data_type);
		switch(properties[i].data_type)
		{
			case MTP_TYPE_STR:
				poke_string(buffer, ofs, (char*)data);
			break;
			case MTP_TYPE_UINT8:
				poke(buffer, ofs, 1, *((uint8_t*)data));
			break;
			case MTP_TYPE_UINT16:
				poke(buffer, ofs, 2, *((uint16_t*)data));
			break;
			case MTP_TYPE_UINT32:
				poke(buffer, ofs, 4, *((uint32_t*)data));
			break;
			case MTP_TYPE_UINT64:
				poke(buffer, ofs, 4, *((uint64_t*)data) & 0xFFFFFFFF);
				poke(buffer, ofs, 4, *((uint64_t*)data) >> 32);
			break;
			default:
				PRINT_ERROR("objectproplist_element : Unsupported data type : 0x%.4X", properties[i].data_type );
			break;
		}

		return 1;
	}

	return 0;
}

int build_objectproplist_dataset(mtp_ctx * ctx, void * buffer, int maxsize,fs_entry * entry, uint32_t handle,uint32_t format_id, uint32_t prop_code, uint32_t prop_group_code, uint32_t depth)
{
	struct stat entrystat;
	time_t t;
	struct tm lt;
	int ofs,tmp,ret,numberofelements;
	char * path;
	char timestr[32];
	uint32_t tmp_dword;

	ret = -1;
	path = build_full_path(ctx->fs_db, mtp_get_storage_root(ctx, entry->storage_id), entry);

	if(path)
	{
	    ret = stat(path, &entrystat);
	}

	if(ret)
	{
		if(path)
			free(path);
		return 0;
	}

	ofs = 0;

	numberofelements = 0;

	poke(buffer, &ofs, 4, numberofelements);   // Number of elements

	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_STORAGE_ID, handle, &entry->storage_id);

	if(entry->flags & ENTRY_IS_DIR)
		tmp_dword = MTP_FORMAT_ASSOCIATION;
	else
		tmp_dword = MTP_FORMAT_UNDEFINED;

	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_OBJECT_FORMAT, handle, &tmp_dword);

	if(entry->flags & ENTRY_IS_DIR)
		tmp_dword = MTP_ASSOCIATION_TYPE_GENERIC_FOLDER;
	else
		tmp_dword = 0x0000;

	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_ASSOCIATION_TYPE, handle, &tmp_dword);
	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_PARENT_OBJECT, handle, &entry->parent);
	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_OBJECT_SIZE, handle, &entry->size);

	tmp_dword = 0x0000;
	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_PROTECTION_STATUS, handle, &tmp_dword);

	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_OBJECT_FILE_NAME, handle, entry->name);

	// Date Created (NR) "YYYYMMDDThhmmss.s"
	t = entrystat.st_mtime;
	localtime_r(&t, &lt);
	snprintf(timestr,sizeof(timestr),"%.4d%.2d%.2dT%.2d%.2d%.2d",1900 + lt.tm_year, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_DATE_CREATED, handle, &timestr);

	// Date Modified (NR) "YYYYMMDDThhmmss.s"
	t = entrystat.st_mtime;
	localtime_r(&t, &lt);
	snprintf(timestr,sizeof(timestr),"%.4d%.2d%.2dT%.2d%.2d%.2d",1900 + lt.tm_year, lt.tm_mon + 1, lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
	numberofelements += objectproplist_element(ctx, buffer, &ofs, MTP_PROPERTY_DATE_MODIFIED, handle, &timestr);

	tmp = 0;

	poke(buffer, &tmp, 4, numberofelements);   // Number of elements
	return ofs;
}