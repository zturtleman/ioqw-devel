/*
===========================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Spearmint Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License.  If not, please
request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//===========================================================================
// BSPC configuration file
// Quake3
//===========================================================================

#define PRESENCE_NONE				1
#define PRESENCE_NORMAL				2
#define PRESENCE_CROUCH				4

bbox	//30x30x80
{
	presencetype	PRESENCE_NORMAL
	flags			0x0000
	mins			{-15, -15, -24}
	maxs			{15, 15, 56}
} //end bbox

bbox	//30x30x66
{
	presencetype	PRESENCE_CROUCH
	flags			0x0001
	mins			{-15, -15, -24}
	maxs			{15, 15, 42}
} //end bbox

settings
{
	phys_gravitydirection		{0, 0, -1}
	phys_friction				6
	phys_stopspeed				100
	phys_gravity				800
	phys_waterfriction			1
	phys_watergravity			400
	phys_maxvelocity			280
	phys_maxscoutvelocity		390
	phys_maxwalkvelocity		280
	phys_maxcrouchvelocity		100
	phys_maxswimvelocity		150
	phys_walkaccelerate			10
	phys_airaccelerate			1
	phys_swimaccelerate			4
	phys_maxstep				19
	phys_maxsteepness			0.7
	phys_maxwaterjump			12
	phys_maxbarrier				43
	phys_maxscoutbarrier		73
	phys_jumpvel				200
	phys_jumpvelscout			300
	phys_falldelta5				40
	phys_falldelta10			60
	phys_strafejumping			1
	rs_waterjump				400
	rs_teleport					50
	rs_barrierjump				100
	rs_startcrouch				300
	rs_startwalkoffledge		70
	rs_startjump				300
	rs_rocketjump				500
	rs_bfgjump					500
	rs_jumppad					250
	rs_aircontrolledjumppad		300
	rs_funcbob					300
	rs_startelevator			50
	rs_falldamage5				300
	rs_falldamage10				500
	rs_maxfallheight			512 // 0 means no limit
	rs_maxjumpfallheight		450
	rs_allowladders				0
} //end settings
