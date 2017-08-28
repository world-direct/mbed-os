#ifndef SWMANAGEMENT_H_
#define SWMANAGEMENT_H_

#include <stdint.h>
#include <stddef.h>
#include "mc_res.h"

#ifdef __cplusplus
extern "C" {
#endif

///< Specifies the result of the validation procedure
///< Implementation Note: These values correspond to values hardcoded in the bootloader, so don't alter them!
typedef enum {

	///< There is an image with a valid CRC checksum
	ValidImage = 0,

	///< No signature byte found in metadata. Normally this is a returned if the section is empty.
	NoMetadata = 1,

	///< The specified image size is out of the allowed range (which depends on total flash and linker settings)
	InvalidMetadata = 2,

	///< The CRC validation failed. This may be caused by an invalid or incomplete image.
	InvalidImage = 3,

	///< The CRC validation failed, and the CRC field is 0xFFFFFFFF. This is normally caused by debug build (without ElfFileProcessor has been run on).
	UnverifyableImage = 4,

} image_validation_result;

typedef struct {
	
	///< Returns the total length of the image, as reported by the server.
	size_t image_total_length;
		
	///< returns a pointer to the name of the downloaded image.
	///< only available if at least the first 1k of the image, which contains the metadata section, has been downloaded.
	const char * image_application_name;
	
	///< returns a pointer to the version string of the downloaded image.
	///< only available if at least the first 1k of the image, which contains the metadata section, has been downloaded.
	const char * image_application_version;

	///< returns the value of the commit-id (should use HEX output)
	///< only available if at least the first 1k of the image, which contains the metadata section, has been downloaded.
	uint32_t image_commit_id;
	
	///< returns validation result. This is a not-cached or persistent value.
	image_validation_result image_validation_result;
	
} swmanagement_image_information;

typedef enum {

	Default = 0xFFFFFFFF,
	UpdatePending = 0xFFFFFF00,
	UpdateSuccessfull = 0x00000000

} swmanagement_update_status;

typedef struct {

	swmanagement_image_information boot_image_information;

	swmanagement_image_information update_image_information;
	
	swmanagement_update_status update_status;
	
} swmanagement_status;


/**
* @brief
*	Returns the sw-status by scanning flash
*/
void swmanagement_get_status(swmanagement_status * status);

/**
* @brief
*	Prepares a new image to be downloaded (maybe to drop a partial invalid one)
*	The internal stream position (for swmangement_update_append) is set to zero.
*/
void swmanagement_prepare_new_download(void);

/**
* @brief
*	Appends raw data to the download area.
*/
void swmangement_append_download_data(const void * buffer, size_t buffer_size);

/**
* @brief  
*	Finally applies the update. Validation is performed by this method!
*	The image will only be applied if the status is valid.
*
*/
image_validation_result swmanagement_update_apply(void);

#ifdef __cplusplus
}
#endif


#endif  // SWMANAGEMENT_H_
