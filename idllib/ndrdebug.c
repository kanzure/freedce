#ifdef DEBUG_INTERP

#include <ndrdebug.h>
#include <dce/idlddefs.h>

char *rpc_dbg_ndr_struct_type(idl_byte struct_type)
{
	switch (struct_type)
	{
		case IDL_DT_CONF_STRUCT:
			return "IDL_DT_CONF_STRUCT";
		case IDL_DT_V1_CONF_STRUCT:
			return "IDL_DT_V1_CONF_STRUCT";
	}
	return "<unknown>";
}

char *rpc_dbg_ndr_type(idl_byte type_byte)
{
	switch (type_byte)
	{
		case IDL_DT_CS_SHADOW:
			return "IDL_DT_CS_SHADOW";
		case IDL_DT_BYTE:
			return "IDL_DT_BYTE";
		case IDL_DT_CHAR:
			return "IDL_DT_CHAR";
		case IDL_DT_BOOLEAN:
			return "IDL_DT_BOOLEAN";
		case IDL_DT_DOUBLE:
			return "IDL_DT_DOUBLE";
		case IDL_DT_ENUM:
			return "IDL_DT_ENUM";
		case IDL_DT_FLOAT:
			return "IDL_DT_FLOAT";
		case IDL_DT_SMALL:
			return "IDL_DT_SMALL";
		case IDL_DT_SHORT:
			return "IDL_DT_SHORT";
		case IDL_DT_LONG:
			return "IDL_DT_LONG";
		case IDL_DT_HYPER:
			return "IDL_DT_HYPER";
		case IDL_DT_USMALL:
			return "IDL_DT_USMALL";
		case IDL_DT_ULONG:
			return "IDL_DT_ULONG";
		case IDL_DT_UHYPER:
			return "IDL_DT_UHYPER";
		case IDL_DT_ERROR_STATUS:
			return "IDL_DT_ERROR_STATUS";
		case IDL_DT_V1_ENUM:
			return "IDL_DT_V1_ENUM";
		case IDL_DT_FIXED_ARRAY:
			return "IDL_DT_FIXED_ARRAY";
		case IDL_DT_VARYING_ARRAY:
			return "IDL_DT_VARYING_ARRAY";
		case IDL_DT_CONF_ARRAY:
			return "IDL_DT_CONF_ARRAY";
		case IDL_DT_OPEN_ARRAY:
			return "IDL_DT_OPEN_ARRAY";
		case IDL_DT_ENC_UNION:
			return "IDL_DT_ENC_UNION";
		case IDL_DT_N_E_UNION:
			return "IDL_DT_N_E_UNION";
		case IDL_DT_FULL_PTR:
			return "IDL_DT_FULL_PTR";
		case IDL_DT_UNIQUE_PTR:
			return "IDL_DT_UNIQUE_PTR";
		case IDL_DT_REF_PTR:
			return "IDL_DT_REF_PTR";
		case IDL_DT_IGNORE:
			return "IDL_DT_IGNORE";
		case IDL_DT_STRING:
			return "IDL_DT_STRING";
		case IDL_DT_TRANSMIT_AS:
			return "IDL_DT_TRANSMIT_AS";
		case IDL_DT_REPRESENT_AS:
			return "IDL_DT_REPRESENT_AS";
		case IDL_DT_INTERFACE:
			return "IDL_DT_INTERFACE";
		case IDL_DT_V1_ARRAY:
			return "IDL_DT_V1_ARRAY";
		case IDL_DT_V1_STRING:
			return "IDL_DT_V1_STRING";
		case IDL_DT_CS_TYPE:
			return "IDL_DT_CS_TYPE";
		case IDL_DT_CS_ATTRIBUTE:
			return "IDL_DT_CS_ATTRIBUTE";
		case IDL_DT_CS_ARRAY:
			return "IDL_DT_CS_ARRAY";
		case IDL_DT_CS_RLSE_SHADOW:
			return "IDL_DT_CS_RLSE_SHADOW";
		case IDL_DT_NDR_ALIGN_2:
			return "IDL_DT_NDR_ALIGN_2";
		case IDL_DT_NDR_ALIGN_4:
			return "IDL_DT_NDR_ALIGN_4";
		case IDL_DT_NDR_ALIGN_8:
			return "IDL_DT_NDR_ALIGN_8";
		case IDL_DT_BEGIN_NESTED_STRUCT:
			return "IDL_DT_BEGIN_NESTED_STRUCT";
		case IDL_DT_END_NESTED_STRUCT:
			return "IDL_DT_END_NESTED_STRUCT";
		case IDL_DT_EOL:
			return "IDL_DT_EOL";
	}
	return "<unknown>";
}

void rpc_dbg_ndr_data(void *type_byte, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		RPC_DBG_NDR_ADD(("%02x ", ((unsigned char*)type_byte)[i]));
}

#endif
