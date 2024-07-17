#ifndef BRANCH_H
#define BRANCH_H

#define LIKELY( e )   __builtin_expect( !!( e ), 1 )
#define UNLIKELY( e ) __builtin_expect( !!( e ), 0 )

#endif
