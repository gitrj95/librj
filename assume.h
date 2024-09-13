#pragma once

#define ASSUME( e )                                                            \
  while( !( e ) ) __builtin_unreachable()