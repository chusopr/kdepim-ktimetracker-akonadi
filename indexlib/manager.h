#ifndef LPC_MANAGER_H1103129409_INCLUDE_GUARD_
#define LPC_MANAGER_H1103129409_INCLUDE_GUARD_

/* This file is part of indexlib.
 * Copyright (C) 2005 Lu�s Pedro Coelho <luis@luispedro.org>
 *
 * Indexlib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation and available as file
 * GPL_V2 which is distributed along with indexlib.
 * 
 * Indexlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston,
 * MA 02110-1301, USA
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of this program with any edition of
 * the Qt library by Trolltech AS, Norway (or with modified versions
 * of Qt that use the same license as Qt), and distribute linked
 * combinations including the two.  You must obey the GNU General
 * Public License in all respects for all of the code used other than
 * Qt.  If you modify this file, you may extend this exception to
 * your version of the file, but you are not obligated to do so.  If
 * you do not wish to do so, delete this exception statement from
 * your version.
 */


class memory_manager  {
	public:
		virtual ~memory_manager() { }
		virtual const unsigned char* ronly_base( unsigned ) const = 0;
		virtual unsigned char* rw_base( unsigned ) const = 0;
		virtual unsigned size() const = 0;
		virtual void resize( unsigned ) = 0;
};

template <memory_manager* ( *get_parent )()>
struct thing_manager {
	public:

		const unsigned char* ronly_base( unsigned idx ) const { return get_parent()->ronly_base( idx ); }
		unsigned char* rw_base( unsigned idx ) const { return get_parent()->rw_base( idx ); }
};
		


#endif /* LPC_MANAGER_H1103129409_INCLUDE_GUARD_ */
