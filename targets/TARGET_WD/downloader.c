#include "downloader.h"
#include "blsrv.h"

static size_t m_offset;

MC_RES downloader_get_status(downloader_status * status)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_validate_update_image;
	desc.args.validate_update_image.command_word = 0;	// no command-word, just validation

	int res = blsrv_call(&desc);
	status->image_validation_result = res; 
	if(res == 0 || res >= 3){
		
		desc.operation = blsrv_get_update_metadata_ptr;
		blsrv_call(&desc);
		intptr_t md = desc.args.get_update_metadata_ptr.start;

		// TODO: we really need to pass this from the bootloader....
		status->image_total_length = *((size_t*)(md + 0x04));
		status->image_application_name = (const char *)(md + 0x08);
		status->image_application_version = (const char *)(md + 0x28);
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

	struct blsrv_desc desc;
	desc.operation = blsrv_write_update_region;
	desc.args.write_update_region.src = src;
	desc.args.write_update_region.offset = m_offset;
	desc.args.write_update_region.size = buffer_size;

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