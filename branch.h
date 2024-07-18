#pragma once

#define LIKELY( e )   __builtin_expect( !!( e ), 1 )
#define UNLIKELY( e ) __builtin_expect( !!( e ), 0 )
