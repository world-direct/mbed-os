#ifndef DOWNLOADER_H_
#define DOWNLOADER_H_

#include <stdint.h>
#include <stddef.h>
#include "mc_res.h"

typedef struct {
	
	///< Returns the length of the data downloaded for the current image.
	size_t image_current_length;	
	
	///< returns a pointer to the name of the downloaded image.
	///< only available if at least the first 1k of the image, which contains the metadata section, has been downloaded.
	const char * image_application_name;
	
	///< returns a pointer to the version string of the downloaded image.
	///< only available if at least the first 1k of the image, which contains the metadata section, has been downloaded.
	const char * image_application_version;
	
	///< returns the crc32 of the raw data in the image, up to the \see image_current_length.
	///< we use the default ETHERNET polynomial of 0xEDB88320.
	uint32_t image_current_crc32;
	
} downloader_status;


/**
* @brief
*	Returns the status of the current downloader image by scanning the download area in flash
*/
MC_RES downloader_get_status(downloader_status * status);

/**
* @brief
*	Prepares a new image to be downloader (maybe to drop a partial invalid one)
*	The internal stream position (for downloader_write) is set to zero.
*/
MC_RES downloader_prepare_new(void);

/**
* @brief
*	Writes raw data to the download area.
*	The data is placed to the position that is returned by downloader_get_status.
*/
MC_RES downloader_append_data(const void * buffer, size_t buffer_size);

/**
* @brief  
*	Finally applies the update. All validation needs to be done before calling this method!
*	You should recheck the download valitity by calling downloader_get_status.
*	NOTE: this function does not return. It forces a reset, where the bootloader will apply the new firmware image.
*/
void downloader_apply(void);


#endif  // DOWNLOADER_H_
