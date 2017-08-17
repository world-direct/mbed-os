#include "downloader.h"
#include "blsrv.h"

static size_t m_offset;

MC_RES downloader_get_status(downloader_status * status)
{
	return MC_RES_NOTIMPL;
}

MC_RES downloader_prepare_new(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_erase_update_region;

	blsrv_call(&desc);
	
	m_offset = 0;

	return MC_RES_OK;
}

MC_RES downloader_append_data(const void * buffer, size_t buffer_size)
{
	return MC_RES_NOTIMPL;
}

void downloader_apply(void)
{
	
}