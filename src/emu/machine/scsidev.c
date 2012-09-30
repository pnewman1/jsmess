/***************************************************************************

 scsidev.c - Base class for scsi devices.

***************************************************************************/

#include "emu.h"
#include "scsidev.h"

scsidev_device::scsidev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void scsidev_device::device_start()
{
	save_item( NAME( command ) );
	save_item( NAME( commandLength ) );
	save_item( NAME( phase ) );
}

#define SCSI_SENSE_SIZE				4

void scsidev_device::ExecCommand( int *transferLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
	case SCSI_CMD_TEST_UNIT_READY:
		SetPhase( SCSI_PHASE_STATUS );
		*transferLength = 0;
		break;

	case SCSI_CMD_RECALIBRATE:
		SetPhase( SCSI_PHASE_STATUS );
		*transferLength = 0;
		break;

	case SCSI_CMD_REQUEST_SENSE:
		SetPhase( SCSI_PHASE_DATAOUT );
		*transferLength = SCSI_SENSE_SIZE;
		break;

	case SCSI_CMD_SEND_DIAGNOSTIC:
		SetPhase( SCSI_PHASE_DATAOUT );
		*transferLength = ( command[ 3 ] << 8 ) + command[ 4 ];
		break;

	default:
		logerror( "%s: SCSIDEV unknown command %02x\n", machine().describe_context(), command[ 0 ] );
		*transferLength = 0;
		break;
	}
}

void scsidev_device::ReadData( UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
	case SCSI_CMD_REQUEST_SENSE:
		data[ 0 ] = SCSI_SENSE_NO_SENSE;
		data[ 1 ] = 0x00;
		data[ 2 ] = 0x00;
		data[ 3 ] = 0x00;
		break;
	default:
		logerror( "%s: SCSIDEV unknown read %02x\n", machine().describe_context(), command[ 0 ] );
		break;
	}
}

void scsidev_device::WriteData( UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	GetCommand( &command, &commandLength );

	switch( command[ 0 ] )
	{
	case SCSI_CMD_SEND_DIAGNOSTIC:
		break;

	default:
		logerror( "%s: SCSIDEV unknown write %02x\n", machine().describe_context(), command[ 0 ] );
		break;
	}
}

void scsidev_device::SetPhase( int _phase )
{
	phase = _phase;
}

void scsidev_device::GetPhase( int *_phase)
{
	*_phase = phase;
}

void scsidev_device::SetCommand( UINT8 *_command, int _commandLength )
{
	if( _commandLength > sizeof( command ) )
	{
		/// TODO: output an error.
		return;
	}

	memcpy( command, _command, _commandLength );
	commandLength = _commandLength;

	SetPhase( SCSI_PHASE_COMMAND );
}

void scsidev_device::GetCommand( UINT8 **_command, int *_commandLength )
{
	*_command = command;
	*_commandLength = commandLength;
}

int scsidev_device::GetDeviceID()
{
	return scsiID;
}

int scsidev_device::GetSectorBytes()
{
	return 0;
}

void scsidev_device::static_set_deviceid( device_t &device, int _scsiID )
{
	scsidev_device &scsidev = downcast<scsidev_device &>(device);
	scsidev.scsiID = _scsiID;
}

int SCSILengthFromUINT8( UINT8 *length )
{
	if( *length == 0 )
	{
		return 256;
	}

	return *length;
}

int SCSILengthFromUINT16( UINT8 *length )
{
	return ( *(length) << 8 ) | *(length + 1 );
}