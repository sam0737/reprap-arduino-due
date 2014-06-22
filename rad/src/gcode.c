#include "rad.h"
#include <stdlib.h>

void gcodeResetParseContext(parse_context_t* context)
{
  *context = 0;
}

char gcodeFilterCharacter(char c, parse_context_t* context)
{
  if (*context == 0)
  {
    if (c == '(') {
        *context = 1;
        return 0;
    } else if (c == ';') {
      *context = 2;
      return 0;
    }
  } else if (*context == 1 && c == ')') {
    *context = 0;
    return 0;
  } else {
    return 0;
  }

  if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
  return c;
}


static bool_t code_seen(char* start, char code, decode_context_t* context)
{
  *context = strchr(start, code);
  return (*context != NULL);
}

static bool_t code_seen_next(decode_context_t* context)
{
  if (*context == NULL)
    return FALSE;
  *context = strchr(*context + 1, *context[0]);
  return (*context != NULL);
}

static float code_value(decode_context_t* context)
{
  if (*context == NULL) return 0;
  float val;
  val = strtof(*context + 1, NULL);
  if (!isnormal(val)) // Not NaN, Inf, Subnormal
    return 0;
  return val;
}

void gcodeInitializeCommand(PrinterCommand* cmd)
{
  memset(cmd, 0, sizeof(PrinterCommand));
  cmd->line = -1;
  cmd->r_value = NAN;
  cmd->s_value = NAN;
  cmd->p_value = -1;
  cmd->printer.feedrate = NAN;
  cmd->t_value = -1;
  cmd->e_value = NAN;
  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    cmd->axes_value[i] = NAN;
  }
}

bool_t gcodeDecode(PrinterCommand* cmd, char* buf, decode_context_t* decode_context)
{
  gcodeInitializeCommand(cmd);
  int value;

  if (code_seen(buf, 'N', decode_context))
    cmd->line = (int32_t) code_value(decode_context);
  if (code_seen(buf, 'R', decode_context))
    cmd->r_value = code_value(decode_context);
  if (code_seen(buf, 'S', decode_context))
    cmd->s_value = code_value(decode_context);
  if (code_seen(buf, 'P', decode_context))
    cmd->p_value = (int32_t) code_value(decode_context);

  if (code_seen(buf, 'F', decode_context))
  {
    cmd->printer.feedrate = code_value(decode_context);
    if (cmd->printer.feedrate < 1)
      return FALSE;
    cmd->type |= COMMANDTYPE_SyncAction;
  }

  if (code_seen(buf, 'E', decode_context)) {
    cmd->e_value = code_value(decode_context);
    cmd->type |= COMMANDTYPE_SyncAction;
  }

  for (uint8_t i = 0; i < RAD_NUMBER_AXES; i++)
  {
    if (code_seen(buf, machine.kinematics.axes[i].name, decode_context)) {
      cmd->axes_value[i] = code_value(decode_context);
      cmd->type |= COMMANDTYPE_SyncAction;
    } else {
      cmd->axes_value[i] = NAN;
    }
  }

  // Tooling (Hot End).
  if (code_seen(buf, 'T', decode_context)) {
    cmd->t_value = (int8_t) code_value(decode_context);
    if (cmd->t_value < 0 || cmd->t_value >= RAD_NUMBER_EXTRUDERS)
      return FALSE;
    cmd->type |= COMMANDTYPE_SyncAction;
  } else {
    cmd->t_value = -1;
  }

  if (code_seen(buf, 'M', decode_context))
    do {
      switch (value = (int) code_value(decode_context)) {
        /* Power */
        case 80: // ATX On
        case 81: // ATX Off
        case 84: // Motor Idle
        case 0: // Stop
        case 1: // Sleep
        case 2: // End program
        case 17: // Stepper On
        case 18: // Stepper Off
          if (cmd->power) return FALSE;
          switch (value) {
          case 81: case 0: value = POWERMODE_Off; break;
          case 2: case 1: value = POWERMODE_Sleep; break;
          case 80: case 17: value = POWERMODE_On; break;
          case 84: case 18: value = POWERMODE_Idle; break;
          }
          cmd->type |= COMMANDTYPE_SyncAction;
          break;

        /* Distance */
        case 82: // Extruder distance
        case 83:
          if (cmd->printer.extruder_distance) return FALSE;
          cmd->printer.extruder_distance = value == 82 ? DISTANCEMODE_Absolute : DISTANCEMODE_Relative;
          cmd->type |= COMMANDTYPE_SyncAction;
          break;

        /* Wait */
        case 116: // Wait all temps
          if (cmd->wait) return FALSE;
          cmd->wait = WAITMODE_All;
          cmd->type |= COMMANDTYPE_SyncAction | COMMANDTYPE_TimeStart;
          break;
        case 109: // Set current tool temp and wait
          if (cmd->wait) return FALSE;
          if (!isnan(cmd->s_value)) {
            if (cmd->code) return FALSE;
            cmd->code = 10104;
          }
          cmd->wait = WAITMODE_CurrentTool;
          cmd->type |= COMMANDTYPE_SyncAction | COMMANDTYPE_TimeStart;
          break;
        case 190: // Set bed temp and wait
          if (cmd->wait) return FALSE;
          if (!isnan(cmd->s_value)) {
            if (cmd->code) return FALSE;
            cmd->code = 10140;
          }
          cmd->wait = WAITMODE_HeatedBed;
          cmd->type |= COMMANDTYPE_SyncAction | COMMANDTYPE_TimeStart;
          break;

        /* Code with immediate effect, handled in the fetcher */
        case 105: // Get temp report
        case 112: // Estop
        case 114: // Get position
        case 115: // Capabilities
        case 111: // Debug capabilities (Ignored for now)
        case 999: // Clear Estop
          if (cmd->code) return FALSE;
          cmd->code = value + 10000;
          break;

        /* Code that is conflict with everything else */
        case 104: // Set current tool temp
        case 140: // Set bed temp
        case 110: // Line number - flush the printer queue
        case 106: // Fan speed
        case 220: // Set feedrate multiplier (speed factor override)
        case 221: // Set flow multiplier (extrude factor override)
          if (cmd->code) return FALSE;
          cmd->code = value + 10000;
          // All of the above are not sync action incidentally
          cmd->type |= COMMANDTYPE_Action;
          break;
        default:
          if (cmd->code) return FALSE;
          cmd->code = value + 10000;
          cmd->type |= COMMANDTYPE_UnknownCode;
          break;
      }
      switch (value) {
        case 104: // Set current tool temp
        case 106: // Fan speed
        case 109: // Set current tool temp and wait
        case 140: // Set bed temp
        case 190: // Set bed temp and wait
        case 220: // Set feedrate multiplier (speed factor override)
        case 221: // Set flow multiplier (extrude factor override)
          if (isnan(cmd->s_value)) return FALSE;
          break;
        case 28:
          return TRUE;
      }
    } while (code_seen_next(decode_context));

  if (code_seen(buf, 'G', decode_context))
    do {
      switch (value = (int) code_value(decode_context)) {
        case 20: // mm
        case 21: // inch
          if (cmd->printer.unit) return FALSE;
          cmd->printer.unit = value == 21 ? UNITMODE_Millimeter : UNITMODE_Inch;
          break;
        case 90: // Absolute
        case 91: // Relative
          if (cmd->printer.distance) return FALSE;
          cmd->printer.distance = value == 90 ? DISTANCEMODE_Absolute : DISTANCEMODE_Relative;
          break;
        case 4: // DWell
          if (cmd->code) return FALSE;
          if (isnan(cmd->p_value)) return FALSE;
          cmd->code = value;
          break;
        case 10: // Offset (Head configuration)
          if (cmd->code) return FALSE;
          if (isnan(cmd->p_value)) return FALSE;
          if (cmd->type & COMMANDTYPE_CanHaveAxisWords) return FALSE;
          cmd->type |= COMMANDTYPE_CanHaveAxisWords;
          cmd->code = value;
          break;
        case 0: // Motion (We treat G0 as G1)
        case 1:
          if (cmd->printer.rapid) return FALSE;
          if (cmd->type & COMMANDTYPE_CanHaveAxisWords) return FALSE;
          cmd->printer.rapid = value == 0 ? RAPIDMODE_Rapid : RAPIDMODE_Feed;
          cmd->type |= COMMANDTYPE_Movement;
          break;
        case 28: // Homing
        case 92: // Set Position
          if (cmd->code) return FALSE;
          if (cmd->type & COMMANDTYPE_CanHaveAxisWords) return FALSE;
          cmd->type |= COMMANDTYPE_CanHaveAxisWords;
          cmd->code = value;
          break;
        default:
          if (cmd->code) return FALSE;
          cmd->code = value;
          cmd->type |= COMMANDTYPE_UnknownCode;
      }
      if (!(cmd->type & COMMANDTYPE_UnknownCode))
        cmd->type |= COMMANDTYPE_SyncAction;
    } while (code_seen_next(decode_context));
  return TRUE;
}
