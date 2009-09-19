/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 * 
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "App.h"

#include "Common.h"
#include "HostGui.h"

#include "GS.h"

StartupParams g_Startup;

//////////////////////////////////////////////////////////////////////////////////////////
// Save Slot Detection System

bool States_isSlotUsed(int num)
{
	if (ElfCRC == 0)
		return false;
	else
		return wxFileExists( SaveStateBase::GetFilename( num ) );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Save state load-from-file (or slot) helpers.

// returns true if the new state was loaded, or false if nothing happened.
void States_Load( const wxString& file )
{
	try
	{
		SysLoadState( file );
		HostGui::Notice( wxsFormat( _("Loaded State %s"),
			wxFileName( file ).GetFullName().c_str() ) );
	}
	catch( Exception::BadSavedState& ex)
	{
		// At this point we can return control back to the user, no questions asked.
		// StateLoadErrors are only thorwn if the load failed prior to any virtual machine
		// memory contents being changed.  (usually missing file errors)

		Console::Notice( ex.FormatDiagnosticMessage() );
	}
	catch( Exception::BaseException& )
	{
		// VM state is probably ruined.  We'll need to recover from the in-memory backup.
		StateRecovery::Recover();
	}

	SysExecute();
}

void States_Load(int num)
{
	wxString file( SaveStateBase::GetFilename( num ) );

	if( !wxFileExists( file ) )
	{
		Console::Notice( "Savestate slot %d is empty.", num );
		return;
	}

	Console::Status( "Loading savestate from slot %d...", num );
	States_Load( file );
	HostGui::Notice( wxsFormat( _("Loaded State (slot %d)"), num ) );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Save state save-to-file (or slot) helpers.

void States_Save( const wxString& file )
{
	if( !EmulationInProgress() )
	{
		Msgbox::Alert( _("You need to start emulation first before you can save it's state.") );
		return;
	}

	try
	{
		Console::Status( wxsFormat( L"Saving savestate to file: %s", file.c_str() ) );
		StateRecovery::SaveToFile( file );
		HostGui::Notice( wxsFormat( _("State saved to file: %s"), wxFileName( file ).GetFullName().c_str() ) );
	}
	catch( Exception::BaseException& ex )
	{
		// TODO: Implement a "pause the action and issue a popup" thing here.
		// *OR* some kind of GS overlay... [for now we use the console]

		// Translation Tip: "Your emulation state has not been saved!"

		/*Msgbox::Alert(
			wxsFormat( _("Error saving state to file: %s"), file.c_str() ) +
			L"\n\n" + _("Error details:") + ex.DisplayMessage()
		);*/

		Console::Error( wxsFormat(
			L"An error occurred while trying to save to file %s\n", file.c_str() ) +
			L"Your emulation state has not been saved!\n"
			L"\nError: " + ex.FormatDiagnosticMessage()
		);
	}
}

void States_Save(int num)
{
	Console::Status( "Saving savestate to slot %d...", num );
	States_Save( SaveStateBase::GetFilename( num ) );
}
