#pragma once

#include <iostream>
#include <vector>

#include "tileNode.h"
#include "forceManager.h"

// Solves collisions between single forces and degenerate corners.
// the forces will simply be inverted.
// * Used in tick state A only.
struct DgenCollisionSolver
{
	// the array is two the two force component pairs that MUST BOTH be set to true in order
	// to be inverted by this solver.  The solver will use these indices to find the other two
	// indices that need flipping as well.
	// X______,    X______,
	// | ,__  |	   |  . . |
	// | |`.  | -> |  _\| |
	// |______|	   |______|

	int forceListIndices[2];
};

// Solves collisions between orthogonally touching entities.
// * Used in tick state A and B.
struct OrthCollisionSolver
{
	// This array is a set of indices into the collision solver's force list, which designates
	// the forces tied to entities, telling them where to move to every tick.  There are 4 indices
	// because a collision only happens when one entity is moving and the other entity is in the way
	// and NOT moving in the same direction.  So each 'left' component index points to the force
	// components in both entities pointing in the same direction (designated as left here 
	// arbitraraly), and each 'right' component is the same but for the pair pointing in the
	// opposite direciton.  See the illustration for an example:
	// ,______,______,   ,______,______,   ,______,______,
	// |      |      |   |      |      |   |      |      |
	// | <  > | <  > | = | L  R | L  R | = | 2  0 | 3  1 |
	// |______|______|   |______|______|   |______|______|
	//      Forces            Pairs            Indices
	int forceListIndices[4];
};

// Solves collisions between diagonally touching entities.
// * Used in tick state A and B.
struct DiagCollisionSolver
{
	// This array is a set of indices into the collision solver's force list, which designates
	// the forces tied to entities, telling them where to move to every tick.  There are 8 indices
	// because diagonal collisions require more information to solve.  A diagonally moving entity
	// has two non-zero orthogonally pointing force components.  So if two diagonally moving entities
	// are colliding, both sets of components need to be checked as well as making sure that there
	// are no components negating the collision.  Keeping the below illustration in mind, if components
	// 0, 2, 5, and 7 were all active forces, while 1, 3, 4, and 6 were inactive, it would describe
	// two entities going in opposite diagonal directions hitting each other.  If, on the other hand,
	// components 4, 5, 6, and 7 were active and 0, 1, 2, 3 were inactive, both entities would be heading
	// in the same direction, making a collision solve unecessary!
	// See the illustration for an example:
	// ,________,             ,_________,             ,_________,
	// |   /\   |             |    U    |             |    6    |
	// | <    > |             |  L   R  |             |  4   0  |
	// |   \/   |             |    D    |             |    2    |
	// |________|_________,   |_________|_________,   |_________|_________,
	//           |   /\   | =           |    U    | =           |    7    |
	//           | <    > |             |  L   R  |             |  5   1  |
	//           |   \/   |             |    D    |             |    3    |
	//           |________|             |_________|             |_________|
	//        Forces                   Pairs                  Indices
	//
	// Unintuative diagonal collisions and their solutions:
	// ,______,           ,______,          
	// |      |           |  /\  |          
	// |   >  |           |      |          
	// |______|______, -> |______|______,
	//        |  /\  |           |      |
	//        |      |           |   >  |
	//        |______|           |______|
	// ,______,           ,______,          
	// |      |           |  /\  |          
	// |   >  |           | <    |          
	// |______|______, -> |______|______,
	//        |  /\  |           |      |
	//        | <    |           |   >  |
	//        |______|           |______|
	//
	int forceListIndices[8];
};

// Solves an edge case of collisions between diagonally touching entities, diagonally moving entities collide
// with other diagonally moving or static entities.  
// * Collisions between diagonally moving entities and orthogonally moving entities are NOT resolved by this type 
//      of solver, as they will be solved by other types!
// * NOT TO BE USED ON THE FIRST SUB TICK OF A SOLVE CYCLE!
// * Used in tick state A and B.
struct PeekCollisionSolver
{
	// The restricted collision solver exists for an edge case like below, where there is one diagonally
	// moving entity whos orthogonal neighbors are not colliding, but it is colliding with a diagonal neighbor.
	// This collision cannot be solved by a regular DiagonalCollisionSolver because those solve cases involving
	// orthogonally moving entities.  If that type of collision was solved here, the forces would seem to phase
	// through the orthogonal neighbor entities, which would be bad!
	// tldr: in cases where there is a 2x2 square of entities, two diagonal collision solvers are needed, but they
	// also cannot solve for orthogonal collisions.
	// ,______,______,    ,______,______,
	// |      |      |    |      | __,  |          
	// |  /\  |      |    |  /\  |   |  |          
	// |______|______|    |______|______|
	// |  __, |      | -> |      |      |
	// |    | |   >  |    |      |   >  |
	// |______|______|    |______|______|
	// *Note: diagonal collisions (and restricteds) are solved AFTER orthogonal collisions, so it should not be possible
	// for a restricted solver to 'steal' a collision from an orthogonal solver.
	// *Note: the forceListIndices are listed in the style of a diagonal collision solver.
	int forceListIndices[8];
};

// Solves collisions that, by their nature, involve 2, or 3 entities.
// * Used in tick state A only.
struct TriACollisionSolver
{
	//    Below are two possible tri-collisions, for example.  These collisions are a little less intuative than the other
	// possible tick state A collisions.
	//        ,______,                 ,______,
	//        |      |		           |      |
	//        |      |		           |  \/  |
	// ,______|______|______,   ,______|______|______,
	// | __,  |      |  ,__ |   |      |      |      |
	// |   |  |      |  |   | , |   >  |      |  <   |
	// |______|      |______|   |______|      |______|
	//    And their respective solutions:
	//        ,______,                 ,______,
	//        |      |		           |      |
	//        |  /\  |		           |  \/  |
	// ,______|______|______,   ,______|______|______,
	// |      |newly |      |   |      |      |      |
	// |  <   |filled|   >  | , |   <  |      |  >   |
	// |______|______|______|   |______|      |______|
	//    The second case is actually pretty suprising, as it is the only collision which results in the creation of
	// a new entity.  There are 4 force components in the initial state, and only three in the final state, so that
	// extra force is converted into an entity and placed in a position that it must be able to add itself to.
	// Generating this new entity will result in the deletion of this OrthagonalTriCollisionSolver however, replacing
	// it with 3 or 4 OrthagonalCollisionSolvers and 2, 3, or 4 Diagonal/PeekCollisionSolvers, depending on if there
	// are or are not entities below.
	//    Because there are now 3 entities, the solver must take 12 components into account when solving the collision.
	// They are ordered in 'right', 'down', 'left', and 'up' order, as the following indices:
	//        ,______,		 
	//        |  11  |		 
	//        |08  02|		 
	// ,______|__05__|______,
	// |  10  |      |  09  |
	// |07  01|      |06  00|
	// |__04__|      |__03__|
	int forceListIndices[12];
};

// Solves collisions involving 2 or 3 entities where one of the entities is offset from the other 1 (or 2) entity(s).
// * Used in tick state B only.
struct TriBCollisionSolver
{
	// Below are 3 examples of tri-collisions on tick state B:
	//    ,_____,	      ,_____,	      ,_____,		    
	//    |     |	      |     |	      |     |		    
	//    |  V  |	      |  ^  |	      |  V  |		    
	// ,__|_____|__, , ,__|_____|__, , ,__|_____|__,
	// | __, | ,__ |   | __, | ,__ |   | ,__ | ,__ |
	// |   | | |   |   |   | | |   |   | |   | |   |
	// |_____|_____|   |_____|_____|   |_____|_____|
	// And their solutions:
	//    ,_____,	      ,_____,	      ,_____,	
	//    |     |	      |     |	      |     |	
	//    |  ^  |	      |  ^  |	      |  ^  |	
	// ,__|_____|__, , ,__|_____|__, , ,__|_____|__,
	// | ,   |   , |   | ,__ | __, |   | ,   | ,   |
	// | |__ | __| |   | |   |   | |   | |__ | |__ |
	// |_____|_____|   |_____|_____|   |_____|_____|
	// As it is impossible to hae a static entity collide with anything in tick state B, and this type
	// of collision is only possible in tick state B, there can be no static entity in the set of 3
	// entities making up this collision.  It can also be noted that in all cases, the solution involves
	// the single topward entity moving up and the botttom two entities moving down with some differences
	// in horizontal movement.  
	// Ordering is as follows:
	//    ,_____,	
	//    |  11 |	
	//    |8   2|	
	// ,__|__5__|__,
	// |  10 |  9  |
	// |7   1|6   0|
	// |__4__|__3__|	        
	int forceListIndices[12];
};

// Solves collisions that, by their nature, involve 2, 3, or 4 entities.
// * Used in tick state A only.
struct QuadCollisionSolver
{
	//    Below are three possible quad collisions, to get the feel for what they solve for.
	// A collision between four moving entities is more impressive, but this solver should be able
	// to solve simple diagonal collisions and all possible three entity collisions that the
	// OrthogonalTriCollisionSolver solves.
	//        ,______,		           ,______,                 ,______,
	//        |      |		           |      |		            |      |
	//        |  \/  |		           |      |		            |  \/  |
	// ,______|______|______,   ,______|______|______,   ,______|______|______,
	// |      |      |      |   | __,  |      |  ,__ |   |      |      |      |
	// |   >  |empty!|  <   | , |   |  |empty!|  |   | , |   >  |empty!|  <   |
	// |______|______|______|   |______|______|______|   |______|______|______|
	//        |      |		           |      |       		    |      |       
	//        |  /\  |		           |      |       		    |      |       
	//        |______|				   |______|					|______|
	//    And their respective solutions:
	//        ,______,		           ,______,                 ,______,
	//        |      |		           |      |		            |      |
	//        |  /\  |		           |  /\  |		            |  \/  |
	// ,______|______|______,   ,______|______|______,   ,______|______|______,
	// |      |      |      |   |      |newly |      |   |      |      |      |
	// |   <  |empty!|  >   | , |  <   |filled|   >  | , |   <  |empty!|  >   |
	// |______|______|______|   |______|______|______|   |______|______|______|
	//        |      |		           |      |       		    |      |       	
	//        |  \/  |		           |      |       		    |      |       
	//        |______|				   |______|					|______|
	//    Because there are now 4 entities, the solver must take 16 components into account when solving the collision.
	// They are ordered in 'right', 'down', 'left', and 'up' order, as the following indices:
	//        ,______,		 
	//        |  15  |		 
	//        |11  03|		 
	// ,______|__07__|______,
	// |  14  |      |  12  |
	// |10  02|empty!|08  00|
	// |__06__|______|__04__|
	//        |  13  |		 
	//        |09  01|		 
	//        |__05__|
	int forceListIndices[16];
};