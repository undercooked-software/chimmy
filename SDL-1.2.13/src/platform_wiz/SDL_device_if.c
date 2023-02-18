/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
//#include <sys/ioctl.h>
#include <fcntl.h>

#include "SDL_config.h"
#include "SDL_wiz_dev.h"

#define BATT_DEV 	"/dev/misc/pollux_batt"
#define BATT_DEV_WITH_DEVFS "/dev/pollux_batt"

struct SDL_WizDevif WizDevif;

int SDL_WizDevIfInit(Uint32 init)
{
	//printf("SDL_WizDevIfInit(init=0x%x)\n", init);
#if 0 /* modified by ikari 2010.3.3 */
	if( ( (init & 0xff) & SDL_WIZ_BATTRY_INIT) == SDL_WIZ_BATTRY_INIT )
	{

		WizDevif.batt_fd = open(BATT_DEV, (O_RDWR | ( (init >>16) & 0xffff))  );

		if(WizDevif.batt_fd < 0){
			WizDevif.batt_fd = open(BATT_DEV_WITH_DEVFS, (O_RDWR | ( (init >>16) & 0xffff))  );

			if (WizDevif.batt_fd < 0){
			    printf("open battery driver failed\n");
			    return -1 ;
			}
		}               
	}
#endif

	if( ( (init & 0xff) & SDL_WIZ_BATTRY_INIT) == SDL_WIZ_BATTRY_INIT )
	{
		WizDevif.batt_fd = open(BATT_DEV, (O_RDONLY | ( (init >>16) & 0xffff)));

		if(WizDevif.batt_fd < 0){
			WizDevif.batt_fd = open(BATT_DEV_WITH_DEVFS, (O_RDONLY | ( (init >>16) & 0xffff)));

			if (WizDevif.batt_fd < 0){
				printf("open battery driver failed\n");
				return -1 ;
			}
		}
	}
	return 0;
}

int SDL_GetBattryCheck(Uint16 *battry_status)
{
    if( WizDevif.batt_fd < 0 ) return -1;
    
    //read(WizDevif.batt_fd, (char *)battry_status, sizeof(Uint16));
    read(WizDevif.batt_fd, battry_status, sizeof(Uint16));
    
    return 0;
}

int SDL_WizDevIfQuit(void)
{
	if(WizDevif.batt_fd>=0)
		close(WizDevif.batt_fd);
	return 0;
}

