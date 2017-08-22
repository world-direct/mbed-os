#include "swmanagement.h"
#include "blsrv.h"


static size_t m_download_offset;

void swmanagement_get_status(swmanagement_status * status)
{
	struct blsrv_desc desc;

	// validate boot image
	desc.operation = blsrv_validate_boot_image;
	blsrv_call(&desc);

	status->update_image_information.image_validation_result = desc.args.validate_update_image.validation_result;

	// we can set the MD pointers directly
	status->update_image_information.image_total_length = *((size_t*)(0x08004300 + 0x04));
	status->update_image_information.image_application_name = (const char *)(0x08004300 + 0x08);
	status->update_image_information.image_application_version = (const char *)(0x08004300 + 0x28);


	//desc.operation = blsrv_validate_update_image;
//
	//blsrv_call(&desc);
	//status->update_image_information.image_validation_result = desc.args.validate_update_image.validation_result;
//
	//// set 
//
	//status->image_validation_result = res; 
	//if(res == 0 || res >= 3){
		//
		//desc.operation = blsrv_get_update_metadata_ptr;
		//blsrv_call(&desc);
		//intptr_t md = desc.args.get_update_metadata_ptr.start;
//
		//// TODO: we really need to pass this from the bootloader....
		//status->image_total_length = *((size_t*)(md + 0x04));
		//status->image_application_name = (const char *)(md + 0x08);
		//status->image_application_version = (const char *)(md + 0x28);
	//} else {
		//status->image_application_name = NULL;
		//status->image_application_version = NULL;
		//status->image_total_length = NULL;
	//}
	//
	//return MC_RES_OK;
}

void swmanagement_prepare_new_download(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_erase_update_region;

	blsrv_call(&desc);
	
	m_download_offset = 0;
}

void swmangement_append_download_data(const void * buffer, size_t buffer_size)
{
	intptr_t src = (intptr_t)buffer;

	struct blsrv_desc desc;
	desc.operation = blsrv_write_update_region;
	desc.args.write_update_region.src = src;
	desc.args.write_update_region.offset = m_download_offset;
	desc.args.write_update_region.size = buffer_size;

	m_download_offset += buffer_size;

	blsrv_call(&desc);

}

void swmanagement_update_apply(void)
{
	struct blsrv_desc desc;
	desc.operation = blsrv_apply_update_with_reset;
	blsrv_call(&desc);
}