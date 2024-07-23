#pragma once

#define EXPECT( e )                                                            \
  while( !( e ) ) __builtin_unreachable()
