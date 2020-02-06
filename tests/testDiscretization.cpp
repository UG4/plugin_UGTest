#include <boost/test/test_case_template.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include "ug.h"

#include "common/common.h"
#include "lib_disc/lib_disc.h"
#include "common/util/typename.h"

using namespace std;
using namespace ug;
using namespace boost::unit_test;


template<int dim, int p>
number func1(const MathVector<dim>& x) {
	number res = 0.;
	for (size_t d = 0; d < dim; ++d)
		res += x[d];// pow(x[d], p) + 0.5;
	return res;
}

template<int dim, int p>
MathVector<dim> Dfunc1(const MathVector<dim>& x) {
	MathVector<dim> res;
	for (size_t d = 0; d < dim; ++d)
		res[d] = 1.0;//p * pow(x[d], p-1);

	return res;
}

template<typename TRefElem, int p>
void TestLagrangeSpacesElem(number& maxDiff) {
	static const double Small = 1e-7;

	LagrangeLSFS<TRefElem, p> space;
	typedef LagrangeLSFS<TRefElem, p> type;
	size_t order = type::order;
	static const int dim__ = type::dim;
	size_t nsh = type::nsh;

	BOOST_MESSAGE("Testing Reference Element of Order " << order
			<< " Dimension:    " << dim__ << " Num Shapes:   " << nsh);

	BOOST_REQUIRE_MESSAGE(nsh == space.num_sh(),
			"Mismatch between type::nsh and num_sh()"<<space.num_sh());

	BOOST_MESSAGE("Multi Indices are: ");
	if (space.num_sh() < 10) {
		for (size_t i = 0; i < space.num_sh(); ++i) {
			MathVector<dim__> pos;
			space.position(i, pos);
			BOOST_MESSAGE("      " << i << ":  " << space.multi_index(i)
					<< " at " << pos);
		}
	}

	//	Multi Indices
	BOOST_CHECKPOINT("Checking MultiIndex mappings ... ");
	for (size_t i = 0; i < space.num_sh(); ++i) {
		MathVector<dim__,int> ind = space.multi_index(i);
		BOOST_REQUIRE_MESSAGE(space.index(ind) == i, "MultiIndex map and back is not identity"
				" for index "<< i << " and MultiIndex " << ind);
	}

	//	Shapes
	BOOST_CHECKPOINT("Checking shapes at dofs are kronecker delta ... ");
	for (size_t i = 0; i < space.num_sh(); ++i) {
		MathVector<dim__> pos;

		//This should be BOOST_WARN_MESSAGE, but doesn't  compile for some reason
		if (!space.position(i, pos)) {
			BOOST_MESSAGE("Cannot get position of shape " << i);
		}

		for (size_t j = 0; j < space.num_sh(); ++j) {
			if (i == j) {
				number diff = fabs(space.shape(j, pos) - 1.0);
				maxDiff = std::max(maxDiff, diff);
				BOOST_REQUIRE_MESSAGE(diff < Small, "Shape "<<j<<" at support point "<<i<<" is"
						" not 1.0 but "<<std::scientific <<space.shape(j, pos));
			} else {
				number diff = fabs(space.shape(j, pos));
				maxDiff = std::max(maxDiff, diff);
				BOOST_REQUIRE_MESSAGE(diff < Small, "Shape "<<j<<" at support point "<<i<<" is"
						" not 0.0 but "<<std::scientific <<space.shape(j, pos));
			}
		}
	}

	//	sum of shapes at arbitrary point
	MathVector<dim__> pos;

	BOOST_CHECKPOINT("Testing Sum of Shapes at arbitrary points ... ");
	for (size_t k = 0; k < 20; ++k) {
		pos[0] = urand(0.0, 1.0);
		if (type::dim >= 2) {
			pos[1] = urand(0.0, 1.0 - pos[0]);
		}
		if (type::dim == 3) {
			pos[2] = urand(0.0, 1.0 - pos[0] - pos[1]);
		}

		std::vector<number> vShape(type::nsh);

		space.shapes(&vShape[0], pos);
		number sum = 0, sum2 = 0;
		for (size_t sh = 0; sh < space.num_sh(); ++sh) {
			sum += space.shape(sh, pos);
			sum2 += vShape[sh];
		}

		number diff = fabs(sum - 1.0);
		maxDiff = std::max(maxDiff, diff);
		BOOST_CHECK_MESSAGE(diff < Small, "Sum of Shapes not 1.0 but differ " << diff);
		BOOST_CHECK_MESSAGE( fabs(sum - sum2) < Small, "Vector sum and sum not equal.");

	}

	BOOST_CHECKPOINT("Testing interpolation of function ... ");

	static const int dim = type::dim;
	// interpolate
	std::vector<number> vInter(type::nsh);
	for (size_t sh = 0; sh < space.num_sh(); ++sh) {
		MathVector<dim__> pos;
		space.position(sh, pos);

		vInter[sh] = func1<dim, p> (pos);
	}

	// eval at arbitrary points
	for (size_t k = 0; k < 20; ++k) {
		pos[0] = urand(0.0, 1.0);
		if (type::dim >= 2) {
			pos[1] = urand(0.0, 1.0 - pos[0]);
		}
		if (type::dim == 3) {
			pos[2] = urand(0.0, 1.0 - pos[0] - pos[1]);
		}

		std::vector<number> vShape(type::nsh);
		std::vector<MathVector<dim> > vGrad(type::nsh);

		space.shapes(&vShape[0], pos);
		space.grads(&vGrad[0], pos);

		number sum = 0, sum2 = 0;
		MathVector<dim> grad, grad2, gradTmp;
		VecSet(grad, 0.0);
		VecSet(grad2, 0.0);
		for (size_t sh = 0; sh < space.num_sh(); ++sh) {
			//			BOOST_MESSAGE(sh << " Adding: int="<<vInter[sh] << ", grad=" << vGrad[sh] << "\n");
			//			BOOST_MESSAGE(sh << " Adding: int="<<vInter[sh] << ", shape=" << vShape[sh] << "\n");
			sum += vInter[sh] * space.shape(sh, pos);
			sum2 += vInter[sh] * vShape[sh];
			space.grad(gradTmp, sh, pos);
			VecScaleAppend(grad, vInter[sh], gradTmp);
			VecScaleAppend(grad2, vInter[sh], vGrad[sh]);
		}

		number diff = fabs(sum - func1<dim, p> (pos));
		maxDiff = std::max(maxDiff, diff);

		BOOST_REQUIRE_MESSAGE(diff < Small, "Sum of Shapes not ok but differ " << diff);
		BOOST_REQUIRE_MESSAGE(fabs(sum - sum2) < Small, "Vector sum and sum not equal.");

		MathVector<dim> vecDiff;
		number diffGrad;

		VecSubtract(vecDiff, grad2, Dfunc1<dim, p> (pos));
		diffGrad = VecTwoNorm(vecDiff);

		MathVector<dim> tmpGrad = Dfunc1<dim, p> (pos);
		BOOST_REQUIRE_MESSAGE(diffGrad < Small, "Sum of Grad (from vector) not ok but differ "
				<< diffGrad << " Grad is " << grad2 << ", exact is " << tmpGrad);

		VecSubtract(vecDiff, grad, Dfunc1<dim, p> (pos));
		diffGrad = VecTwoNorm(vecDiff);
		BOOST_REQUIRE_MESSAGE((diffGrad < Small),
				"Sum of Grad not ok but differ " << diffGrad);
	}
}

BOOST_AUTO_TEST_SUITE( TrialSpaceTestSuiteNumProc1 );

/// the template parameters of mpl::list defines all types
/// to test in the BOOST_AUTO_TEST_CASE_TEMPLATE
typedef boost::mpl::list<ReferenceEdge,
ReferenceQuadrilateral,
ReferenceHexahedron,
ReferenceTriangle,
ReferenceTetrahedron,
ReferencePrism> test_types;

/*
 *	Testing Lagrange Spaces for several orders for the reference elements
 *	defined in the boost::mpl::list "test_types".
 */
BOOST_AUTO_TEST_CASE_TEMPLATE(testLagrangeSpacesElem, T, test_types)
{
	string msg ( "testing Lagrange Shape functions on ");
	msg.append( TypeName<T>() + string(" for order 1, 2, 3."));
	BOOST_MESSAGE(msg);

	number maxDiff = 0.0;

	TestLagrangeSpacesElem<T, 1>(maxDiff);
	TestLagrangeSpacesElem<T, 2>(maxDiff);
	TestLagrangeSpacesElem<T, 3>(maxDiff);
}

/*
 * Testing Lagrange Space of ReferencePyramid for order 1.
 */
BOOST_AUTO_TEST_CASE(testLagrangeSpacesReferencePyramid)
{
	BOOST_MESSAGE("testing Lagrange Shape functions on the Reference Pyramid "
					" for order 1.");

	number maxDiff = 0.0;

	try {
		TestLagrangeSpacesElem<ReferencePyramid, 1>(maxDiff);
	} catch(UGError& e) {
		BOOST_ERROR("exception UGError caught at " <<
				e.get_file(0) << ":" << e.get_line(0) << " msg: "<< e.get_msg());
	}
}

BOOST_AUTO_TEST_SUITE_END();

