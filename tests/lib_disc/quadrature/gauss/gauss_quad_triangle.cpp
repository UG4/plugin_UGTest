/*
 * Copyright (c) 2011-2013:  G-CSC, Goethe University Frankfurt
 * 
 * This file is part of UG4.
 * 
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 * 
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

//  This file is parsed from UG 3.9.
//  It provides the Gauss Quadratures for a reference triangle.


#include "../quadrature.h"
#include "gauss_quad_triangle.h"

namespace ug{

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 1>, 2, 1, 1>::m_vWeight[1] =
{
0.5 * 1.00000000000000000000000000000000
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 1>, 2, 1, 1>::m_vPoint[1] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 2>, 2, 2, 3>::m_vWeight[3] =
{
0.5 * 0.33333333333333333333333333333333,
0.5 * 0.33333333333333333333333333333333,
0.5 * 0.33333333333333333333333333333333
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 2>, 2, 2, 3>::m_vPoint[3] =
{
MathVector<2>(0.16666666666666666666666666666667, 0.16666666666666666666666666666667),
MathVector<2>(0.16666666666666666666666666666667, 0.66666666666666666666666666666667),
MathVector<2>(0.66666666666666666666666666666667, 0.16666666666666666666666666666667)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 3>, 2, 3, 4>::m_vWeight[4] =
{
0.5 * -0.56250000000000000000000000000000,
0.5 * 0.52083333333333333333333333333333,
0.5 * 0.52083333333333333333333333333333,
0.5 * 0.52083333333333333333333333333333
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 3>, 2, 3, 4>::m_vPoint[4] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.20000000000000000000000000000000, 0.20000000000000000000000000000000),
MathVector<2>(0.20000000000000000000000000000000, 0.60000000000000000000000000000000),
MathVector<2>(0.60000000000000000000000000000000, 0.20000000000000000000000000000000)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 4>, 2, 4, 6>::m_vWeight[6] =
{
0.5 * 0.10995174365532186763832632490021,
0.5 * 0.10995174365532186763832632490021,
0.5 * 0.10995174365532186763832632490021,
0.5 * 0.22338158967801146569500700843312,
0.5 * 0.22338158967801146569500700843312,
0.5 * 0.22338158967801146569500700843312
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 4>, 2, 4, 6>::m_vPoint[6] =
{
MathVector<2>(0.09157621350977074345957146340220, 0.09157621350977074345957146340220),
MathVector<2>(0.09157621350977074345957146340220, 0.81684757298045851308085707319560),
MathVector<2>(0.81684757298045851308085707319560, 0.09157621350977074345957146340220),
MathVector<2>(0.44594849091596488631832925388305, 0.44594849091596488631832925388305),
MathVector<2>(0.44594849091596488631832925388305, 0.10810301816807022736334149223390),
MathVector<2>(0.10810301816807022736334149223390, 0.44594849091596488631832925388305)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 5>, 2, 5, 7>::m_vWeight[7] =
{
0.5 * 0.22500000000000000000000000000000,
0.5 * 0.12593918054482715259568394550018,
0.5 * 0.12593918054482715259568394550018,
0.5 * 0.12593918054482715259568394550018,
0.5 * 0.13239415278850618073764938783315,
0.5 * 0.13239415278850618073764938783315,
0.5 * 0.13239415278850618073764938783315
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 5>, 2, 5, 7>::m_vPoint[7] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.10128650732345633880098736191512, 0.10128650732345633880098736191512),
MathVector<2>(0.10128650732345633880098736191512, 0.79742698535308732239802527616975),
MathVector<2>(0.79742698535308732239802527616975, 0.10128650732345633880098736191512),
MathVector<2>(0.47014206410511508977044120951345, 0.47014206410511508977044120951345),
MathVector<2>(0.47014206410511508977044120951345, 0.05971587178976982045911758097311),
MathVector<2>(0.05971587178976982045911758097311, 0.47014206410511508977044120951345)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 6>, 2, 6, 12>::m_vWeight[12] =
{
0.5 * 0.05084490637020681692093680910687,
0.5 * 0.05084490637020681692093680910687,
0.5 * 0.05084490637020681692093680910687,
0.5 * 0.11678627572637936602528961138558,
0.5 * 0.11678627572637936602528961138558,
0.5 * 0.11678627572637936602528961138558,
0.5 * 0.08285107561837357519355345642044,
0.5 * 0.08285107561837357519355345642044,
0.5 * 0.08285107561837357519355345642044,
0.5 * 0.08285107561837357519355345642044,
0.5 * 0.08285107561837357519355345642044,
0.5 * 0.08285107561837357519355345642044
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 6>, 2, 6, 12>::m_vPoint[12] =
{
MathVector<2>(0.06308901449150222834033160287082, 0.06308901449150222834033160287082),
MathVector<2>(0.06308901449150222834033160287082, 0.87382197101699554331933679425836),
MathVector<2>(0.87382197101699554331933679425836, 0.06308901449150222834033160287082),
MathVector<2>(0.24928674517091042129163855310702, 0.24928674517091042129163855310702),
MathVector<2>(0.24928674517091042129163855310702, 0.50142650965817915741672289378596),
MathVector<2>(0.50142650965817915741672289378596, 0.24928674517091042129163855310702),
MathVector<2>(0.05314504984481694735324967163140, 0.31035245103378440541660773395655),
MathVector<2>(0.05314504984481694735324967163140, 0.63650249912139864723014259441205),
MathVector<2>(0.31035245103378440541660773395655, 0.05314504984481694735324967163140),
MathVector<2>(0.31035245103378440541660773395655, 0.63650249912139864723014259441205),
MathVector<2>(0.63650249912139864723014259441205, 0.05314504984481694735324967163140),
MathVector<2>(0.63650249912139864723014259441205, 0.31035245103378440541660773395655)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 7>, 2, 7, 12>::m_vWeight[12] =
{
0.5 * 0.05303405631487250285750836092148,
0.5 * 0.05303405631487250285750836092148,
0.5 * 0.05303405631487250285750836092148,
0.5 * 0.08776281742889211007353980627857,
0.5 * 0.08776281742889211007353980627857,
0.5 * 0.08776281742889211007353980627857,
0.5 * 0.05755008556996317147689099380044,
0.5 * 0.05755008556996317147689099380044,
0.5 * 0.05755008556996317147689099380044,
0.5 * 0.13498637401960554892539417233284,
0.5 * 0.13498637401960554892539417233284,
0.5 * 0.13498637401960554892539417233284
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 7>, 2, 7, 12>::m_vPoint[12] =
{
MathVector<2>(0.06238226509440211817368300099635, 0.06751786707391608544255713105087),
MathVector<2>(0.06751786707391608544255713105087, 0.87009986783168179638375986795278),
MathVector<2>(0.87009986783168179638375986795278, 0.06238226509440211817368300099635),
MathVector<2>(0.05522545665692661173747919027564, 0.32150249385198182266630784919920),
MathVector<2>(0.32150249385198182266630784919920, 0.62327204949109156559621296052515),
MathVector<2>(0.62327204949109156559621296052515, 0.05522545665692661173747919027564),
MathVector<2>(0.03432430294509714646963064248394, 0.66094919618673565761198031019780),
MathVector<2>(0.66094919618673565761198031019780, 0.30472650086816719591838904731826),
MathVector<2>(0.30472650086816719591838904731826, 0.03432430294509714646963064248394),
MathVector<2>(0.51584233435359177925746338682643, 0.27771616697639178256958187139372),
MathVector<2>(0.27771616697639178256958187139372, 0.20644149867001643817295474177985),
MathVector<2>(0.20644149867001643817295474177985, 0.51584233435359177925746338682643)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 8>, 2, 8, 16>::m_vWeight[16] =
{
0.5 * 0.14431560767778716825109111048906,
0.5 * 0.10321737053471825028179155029213,
0.5 * 0.10321737053471825028179155029213,
0.5 * 0.10321737053471825028179155029213,
0.5 * 0.03245849762319808031092592834178,
0.5 * 0.03245849762319808031092592834178,
0.5 * 0.03245849762319808031092592834178,
0.5 * 0.09509163426728462479389610438858,
0.5 * 0.09509163426728462479389610438858,
0.5 * 0.09509163426728462479389610438858,
0.5 * 0.02723031417443499426484469007391,
0.5 * 0.02723031417443499426484469007391,
0.5 * 0.02723031417443499426484469007391,
0.5 * 0.02723031417443499426484469007391,
0.5 * 0.02723031417443499426484469007391,
0.5 * 0.02723031417443499426484469007391
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 8>, 2, 8, 16>::m_vPoint[16] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.17056930775176020662229350149146, 0.17056930775176020662229350149146),
MathVector<2>(0.17056930775176020662229350149146, 0.65886138449647958675541299701707),
MathVector<2>(0.65886138449647958675541299701707, 0.17056930775176020662229350149146),
MathVector<2>(0.05054722831703097545842355059660, 0.05054722831703097545842355059660),
MathVector<2>(0.05054722831703097545842355059660, 0.89890554336593804908315289880680),
MathVector<2>(0.89890554336593804908315289880680, 0.05054722831703097545842355059660),
MathVector<2>(0.45929258829272315602881551449417, 0.45929258829272315602881551449417),
MathVector<2>(0.45929258829272315602881551449417, 0.08141482341455368794236897101166),
MathVector<2>(0.08141482341455368794236897101166, 0.45929258829272315602881551449417),
MathVector<2>(0.72849239295540428124100037917606, 0.26311282963463811342178578628464),
MathVector<2>(0.72849239295540428124100037917606, 0.00839477740995760533721383453930),
MathVector<2>(0.26311282963463811342178578628464, 0.72849239295540428124100037917606),
MathVector<2>(0.26311282963463811342178578628464, 0.00839477740995760533721383453930),
MathVector<2>(0.00839477740995760533721383453930, 0.72849239295540428124100037917606),
MathVector<2>(0.00839477740995760533721383453930, 0.26311282963463811342178578628464)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 9>, 2, 9, 19>::m_vWeight[19] =
{
0.5 * 0.09713579628279883381924198250729,
0.5 * 0.03133470022713907053685483128721,
0.5 * 0.03133470022713907053685483128721,
0.5 * 0.03133470022713907053685483128721,
0.5 * 0.07782754100477427931673935629940,
0.5 * 0.07782754100477427931673935629940,
0.5 * 0.07782754100477427931673935629940,
0.5 * 0.07964773892721025303289177426404,
0.5 * 0.07964773892721025303289177426404,
0.5 * 0.07964773892721025303289177426404,
0.5 * 0.02557767565869803126167879855900,
0.5 * 0.02557767565869803126167879855900,
0.5 * 0.02557767565869803126167879855900,
0.5 * 0.04328353937728937728937728937729,
0.5 * 0.04328353937728937728937728937729,
0.5 * 0.04328353937728937728937728937729,
0.5 * 0.04328353937728937728937728937729,
0.5 * 0.04328353937728937728937728937729,
0.5 * 0.04328353937728937728937728937729
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 9>, 2, 9, 19>::m_vPoint[19] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.48968251919873762778370692483619, 0.48968251919873762778370692483619),
MathVector<2>(0.48968251919873762778370692483619, 0.02063496160252474443258615032762),
MathVector<2>(0.02063496160252474443258615032762, 0.48968251919873762778370692483619),
MathVector<2>(0.43708959149293663726993036443535, 0.43708959149293663726993036443535),
MathVector<2>(0.43708959149293663726993036443535, 0.12582081701412672546013927112929),
MathVector<2>(0.12582081701412672546013927112929, 0.43708959149293663726993036443535),
MathVector<2>(0.18820353561903273024096128046733, 0.18820353561903273024096128046733),
MathVector<2>(0.18820353561903273024096128046733, 0.62359292876193453951807743906533),
MathVector<2>(0.62359292876193453951807743906533, 0.18820353561903273024096128046733),
MathVector<2>(0.04472951339445270986510658996628, 0.04472951339445270986510658996628),
MathVector<2>(0.04472951339445270986510658996628, 0.91054097321109458026978682006745),
MathVector<2>(0.91054097321109458026978682006745, 0.04472951339445270986510658996628),
MathVector<2>(0.74119859878449802069007987352342, 0.03683841205473628363481759878339),
MathVector<2>(0.74119859878449802069007987352342, 0.22196298916076569567510252769319),
MathVector<2>(0.03683841205473628363481759878339, 0.74119859878449802069007987352342),
MathVector<2>(0.03683841205473628363481759878339, 0.22196298916076569567510252769319),
MathVector<2>(0.22196298916076569567510252769319, 0.74119859878449802069007987352342),
MathVector<2>(0.22196298916076569567510252769319, 0.03683841205473628363481759878339)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 10>, 2, 10, 25>::m_vWeight[25] =
{
0.5 * 0.07989450474123970783124704521339,
0.5 * 0.07112380223237733463929128739866,
0.5 * 0.07112380223237733463929128739866,
0.5 * 0.07112380223237733463929128739866,
0.5 * 0.00822381869046419551864662036247,
0.5 * 0.00822381869046419551864662036247,
0.5 * 0.00822381869046419551864662036247,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.04543059229617001800707362924393,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03735985623430527682623649900198,
0.5 * 0.03088665688456398878251307700463,
0.5 * 0.03088665688456398878251307700463,
0.5 * 0.03088665688456398878251307700463,
0.5 * 0.03088665688456398878251307700463,
0.5 * 0.03088665688456398878251307700463,
0.5 * 0.03088665688456398878251307700463
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 10>, 2, 10, 25>::m_vPoint[25] =
{
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.42508621060209057296952951163804, 0.42508621060209057296952951163804),
MathVector<2>(0.42508621060209057296952951163804, 0.14982757879581885406094097672391),
MathVector<2>(0.14982757879581885406094097672391, 0.42508621060209057296952951163804),
MathVector<2>(0.02330886751000019071446638689598, 0.02330886751000019071446638689598),
MathVector<2>(0.02330886751000019071446638689598, 0.95338226497999961857106722620804),
MathVector<2>(0.95338226497999961857106722620804, 0.02330886751000019071446638689598),
MathVector<2>(0.62830740021349255642083766607883, 0.22376697357697300622568649026820),
MathVector<2>(0.62830740021349255642083766607883, 0.14792562620953443735347584365296),
MathVector<2>(0.22376697357697300622568649026820, 0.62830740021349255642083766607883),
MathVector<2>(0.22376697357697300622568649026820, 0.14792562620953443735347584365296),
MathVector<2>(0.14792562620953443735347584365296, 0.62830740021349255642083766607883),
MathVector<2>(0.14792562620953443735347584365296, 0.22376697357697300622568649026820),
MathVector<2>(0.61131382618139764891875500225390, 0.35874014186443146457815530072385),
MathVector<2>(0.61131382618139764891875500225390, 0.02994603195417088650308969702225),
MathVector<2>(0.35874014186443146457815530072385, 0.61131382618139764891875500225390),
MathVector<2>(0.35874014186443146457815530072385, 0.02994603195417088650308969702225),
MathVector<2>(0.02994603195417088650308969702225, 0.61131382618139764891875500225390),
MathVector<2>(0.02994603195417088650308969702225, 0.35874014186443146457815530072385),
MathVector<2>(0.82107206998562937337354441347218, 0.14329537042686714530585663061732),
MathVector<2>(0.82107206998562937337354441347218, 0.03563255958750348132059895591050),
MathVector<2>(0.14329537042686714530585663061732, 0.82107206998562937337354441347218),
MathVector<2>(0.14329537042686714530585663061732, 0.03563255958750348132059895591050),
MathVector<2>(0.03563255958750348132059895591050, 0.82107206998562937337354441347218),
MathVector<2>(0.03563255958750348132059895591050, 0.14329537042686714530585663061732)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 11>, 2, 11, 28>::m_vWeight[28] =
{
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.00736238378330055426425889504738,
0.5 * 0.08797730116223223879809316932146,
0.5 * 0.00874431155373602304951642879983,
0.5 * 0.00874431155373602304951642879983,
0.5 * 0.00874431155373602304951642879983,
0.5 * 0.03808157199393493751502433943561,
0.5 * 0.03808157199393493751502433943561,
0.5 * 0.03808157199393493751502433943561,
0.5 * 0.01885544805613129205847678259112,
0.5 * 0.01885544805613129205847678259112,
0.5 * 0.01885544805613129205847678259112,
0.5 * 0.07215969754473952612402998858646,
0.5 * 0.07215969754473952612402998858646,
0.5 * 0.07215969754473952612402998858646,
0.5 * 0.06932913870553589984176565090381,
0.5 * 0.06932913870553589984176565090381,
0.5 * 0.06932913870553589984176565090381,
0.5 * 0.04105631542928856664165231490729,
0.5 * 0.04105631542928856664165231490729,
0.5 * 0.04105631542928856664165231490729,
0.5 * 0.04105631542928856664165231490729,
0.5 * 0.04105631542928856664165231490729,
0.5 * 0.04105631542928856664165231490729
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 11>, 2, 11, 28>::m_vPoint[28] =
{
MathVector<2>(0.85887028128263670403917393805835, 0.14112971871736329596082606194165),
MathVector<2>(0.85887028128263670403917393805835, 0.00000000000000000000000000000000),
MathVector<2>(0.14112971871736329596082606194165, 0.85887028128263670403917393805835),
MathVector<2>(0.14112971871736329596082606194165, 0.00000000000000000000000000000000),
MathVector<2>(0.00000000000000000000000000000000, 0.85887028128263670403917393805835),
MathVector<2>(0.00000000000000000000000000000000, 0.14112971871736329596082606194165),
MathVector<2>(0.33333333333333333333333333333333, 0.33333333333333333333333333333333),
MathVector<2>(0.02598914092828739526003248549884, 0.02598914092828739526003248549884),
MathVector<2>(0.02598914092828739526003248549884, 0.94802171814342520947993502900232),
MathVector<2>(0.94802171814342520947993502900232, 0.02598914092828739526003248549884),
MathVector<2>(0.09428750264792249563056977627541, 0.09428750264792249563056977627541),
MathVector<2>(0.09428750264792249563056977627541, 0.81142499470415500873886044744919),
MathVector<2>(0.81142499470415500873886044744919, 0.09428750264792249563056977627541),
MathVector<2>(0.49463677501721381374163260230644, 0.49463677501721381374163260230644),
MathVector<2>(0.49463677501721381374163260230644, 0.01072644996557237251673479538713),
MathVector<2>(0.01072644996557237251673479538713, 0.49463677501721381374163260230644),
MathVector<2>(0.20734338261451133345293402411297, 0.20734338261451133345293402411297),
MathVector<2>(0.20734338261451133345293402411297, 0.58531323477097733309413195177407),
MathVector<2>(0.58531323477097733309413195177407, 0.20734338261451133345293402411297),
MathVector<2>(0.43890780570049209506106538163613, 0.43890780570049209506106538163613),
MathVector<2>(0.43890780570049209506106538163613, 0.12218438859901580987786923672775),
MathVector<2>(0.12218438859901580987786923672775, 0.43890780570049209506106538163613),
MathVector<2>(0.67793765488259040154212614118875, 0.04484167758913044330905239146880),
MathVector<2>(0.67793765488259040154212614118875, 0.27722066752827915514882146734245),
MathVector<2>(0.04484167758913044330905239146880, 0.67793765488259040154212614118875),
MathVector<2>(0.04484167758913044330905239146880, 0.27722066752827915514882146734245),
MathVector<2>(0.27722066752827915514882146734245, 0.67793765488259040154212614118875),
MathVector<2>(0.27722066752827915514882146734245, 0.04484167758913044330905239146880)
};

template <>
number GaussQuadBase<GaussQuadrature<ReferenceTriangle, 12>, 2, 12, 33>::m_vWeight[33] =
{
0.5 * 0.02573106644045500000000000000000,
0.5 * 0.02573106644045500000000000000000,
0.5 * 0.02573106644045500000000000000000,
0.5 * 0.04369254453803800000000000000000,
0.5 * 0.04369254453803800000000000000000,
0.5 * 0.04369254453803800000000000000000,
0.5 * 0.06285822421788500000000000000000,
0.5 * 0.06285822421788500000000000000000,
0.5 * 0.06285822421788500000000000000000,
0.5 * 0.03479611293070900000000000000000,
0.5 * 0.03479611293070900000000000000000,
0.5 * 0.03479611293070900000000000000000,
0.5 * 0.00616626105155900000000000000000,
0.5 * 0.00616626105155900000000000000000,
0.5 * 0.00616626105155900000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.04037155776638100000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.02235677320230300000000000000000,
0.5 * 0.01731623110865900000000000000000,
0.5 * 0.01731623110865900000000000000000,
0.5 * 0.01731623110865900000000000000000,
0.5 * 0.01731623110865900000000000000000,
0.5 * 0.01731623110865900000000000000000,
0.5 * 0.01731623110865900000000000000000
};

template <>
MathVector<2> GaussQuadBase<GaussQuadrature<ReferenceTriangle, 12>, 2, 12, 33>::m_vPoint[33] =
{
MathVector<2>(0.02356522045239000000000000000000, 0.48821738977380500000000000000000),
MathVector<2>(0.48821738977380500000000000000000, 0.02356522045239000000000000000000),
MathVector<2>(0.48821738977380500000000000000000, 0.48821738977380500000000000000000),
MathVector<2>(0.43972439229446000000000000000000, 0.43972439229446000000000000000000),
MathVector<2>(0.43972439229446000000000000000000, 0.12055121541107900000000000000000),
MathVector<2>(0.12055121541107900000000000000000, 0.43972439229446000000000000000000),
MathVector<2>(0.27121038501211600000000000000000, 0.27121038501211600000000000000000),
MathVector<2>(0.27121038501211600000000000000000, 0.45757922997576800000000000000000),
MathVector<2>(0.45757922997576800000000000000000, 0.27121038501211600000000000000000),
MathVector<2>(0.12757614554158600000000000000000, 0.12757614554158600000000000000000),
MathVector<2>(0.12757614554158600000000000000000, 0.74484770891682790000000000000000),
MathVector<2>(0.74484770891682790000000000000000, 0.12757614554158600000000000000000),
MathVector<2>(0.02131735045321000000000000000000, 0.02131735045321000000000000000000),
MathVector<2>(0.02131735045321000000000000000000, 0.95736529909357990000000000000000),
MathVector<2>(0.95736529909357990000000000000000, 0.02131735045321000000000000000000),
MathVector<2>(0.11534349453469800000000000000000, 0.27571326968551400000000000000000),
MathVector<2>(0.11534349453469800000000000000000, 0.60894323577978790000000000000000),
MathVector<2>(0.27571326968551400000000000000000, 0.11534349453469800000000000000000),
MathVector<2>(0.27571326968551400000000000000000, 0.60894323577978790000000000000000),
MathVector<2>(0.60894323577978790000000000000000, 0.11534349453469800000000000000000),
MathVector<2>(0.60894323577978790000000000000000, 0.27571326968551400000000000000000),
MathVector<2>(0.02283833222225700000000000000000, 0.28132558098994000000000000000000),
MathVector<2>(0.02283833222225700000000000000000, 0.69583608678780310000000000000000),
MathVector<2>(0.28132558098994000000000000000000, 0.02283833222225700000000000000000),
MathVector<2>(0.28132558098994000000000000000000, 0.69583608678780310000000000000000),
MathVector<2>(0.69583608678780310000000000000000, 0.02283833222225700000000000000000),
MathVector<2>(0.69583608678780310000000000000000, 0.28132558098994000000000000000000),
MathVector<2>(0.02573405054833000000000000000000, 0.11625191590759700000000000000000),
MathVector<2>(0.02573405054833000000000000000000, 0.85801403354407300000000000000000),
MathVector<2>(0.11625191590759700000000000000000, 0.02573405054833000000000000000000),
MathVector<2>(0.11625191590759700000000000000000, 0.85801403354407300000000000000000),
MathVector<2>(0.85801403354407300000000000000000, 0.02573405054833000000000000000000),
MathVector<2>(0.85801403354407300000000000000000, 0.11625191590759700000000000000000)
};




template <>
FlexGaussQuadrature<ReferenceTriangle>::FlexGaussQuadrature(int order)
{
	switch(order)
	{
	case 0:
	case 1:
		m_order = GaussQuadrature<ReferenceTriangle, 1>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 1>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 1>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 1>::weights();
		break;

	case 2:
		m_order = GaussQuadrature<ReferenceTriangle, 2>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 2>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 2>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 2>::weights();
		break;

	case 3:
		m_order = GaussQuadrature<ReferenceTriangle, 3>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 3>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 3>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 3>::weights();
		break;

	case 4:
		m_order = GaussQuadrature<ReferenceTriangle, 4>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 4>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 4>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 4>::weights();
		break;

	case 5:
		m_order = GaussQuadrature<ReferenceTriangle, 5>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 5>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 5>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 5>::weights();
		break;

	case 6:
		m_order = GaussQuadrature<ReferenceTriangle, 6>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 6>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 6>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 6>::weights();
		break;

	case 7:
		m_order = GaussQuadrature<ReferenceTriangle, 7>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 7>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 7>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 7>::weights();
		break;

	case 8:
		m_order = GaussQuadrature<ReferenceTriangle, 8>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 8>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 8>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 8>::weights();
		break;

	case 9:
		m_order = GaussQuadrature<ReferenceTriangle, 9>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 9>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 9>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 9>::weights();
		break;

	case 10:
		m_order = GaussQuadrature<ReferenceTriangle, 10>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 10>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 10>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 10>::weights();
		break;

	case 11:
		m_order = GaussQuadrature<ReferenceTriangle, 11>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 11>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 11>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 11>::weights();
		break;

	case 12:
		m_order = GaussQuadrature<ReferenceTriangle, 12>::order();
		m_numPoints = GaussQuadrature<ReferenceTriangle, 12>::size();
		m_pvPoint = GaussQuadrature<ReferenceTriangle, 12>::points();
		m_pvWeight = GaussQuadrature<ReferenceTriangle, 12>::weights();
		break;

	default: UG_THROW("Order "<<order<<" not available for GaussQuadrature of triangle.");
	}
}
}; // namespace ug

