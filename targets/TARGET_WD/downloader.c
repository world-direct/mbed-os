#include "downloader.h"
#include "blsrv.h"

static size_t m_offset;
extern intptr_t __update_image_start;

MC_RES downloader_get_status(downloader_status * status)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_validate_update_image;
	desc.args.validate_update_image.command_word = 0;	// no command-word, just validation

	int res = blsrv_call(&desc);
	status->image_validation_result = res; 
	if(res == 0 || res >= 3){
		// load metadata fields if the metadata is valid at least
		status->image_total_length = __update_image_start + 0x304;
		status->image_application_name = __update_image_start + 0x308;
		status->image_application_version = __update_image_start + 0x328;
	} else {
		status->image_application_name = NULL;
		status->image_application_version = NULL;
		status->image_total_length = NULL;
	}
	
	return MC_RES_OK;
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
	intptr_t src = (intptr_t)buffer;
	intptr_t dest = __update_image_start + m_offset;

	struct blsrv_desc desc;
	desc.operation = blsrv_flashmemcpy;
	desc.args.flashmemcpy.src = src;
	desc.args.flashmemcpy.dest = dest;
	desc.args.flashmemcpy.size = buffer_size;

	m_offset += buffer_size;

	if(!blsrv_call(&desc))
		return MC_RES_ERROR;

	return MC_RES_OK;

}

void downloader_apply(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_validate_update_image;
	desc.args.validate_update_image.command_word = 0x0000FFFF;
	blsrv_call(&desc);

	// reset
}