/* -*- mode: c++ -*-

   This file is part of the Life library

   Author(s): Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   Date: 2006-12-30

   Copyright (C) 2006-2008 Université Joseph Fourier (Grenoble)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file bdf.hpp
   \author Christophe Prud'homme <christophe.prudhomme@ujf-grenoble.fr>
   \date 2006-12-30
*/
#ifndef _BDF_H
#define _BDF_H

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/timer.hpp>
#include <boost/shared_array.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/utility.hpp>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#include <life/lifecore/life.hpp>
#include <life/lifealg/glas.hpp>

namespace Life
{
namespace ublas = boost::numeric::ublas;
namespace fs = boost::filesystem;

enum BDFState { BDF_UNITIALIZED = 0, BDF_RUNNING, BDF_STOPPED };

enum BDFTimeScheme { BDF_ORDER_ONE=1, BDF_ORDER_TWO, BDF_ORDER_THREE, BDF_ORDER_FOUR, BDF_MAX_ORDER = 4 };
enum BDFStragegy { BDF_STRATEGY_DT_CONSTANT,
                   BDF_STRATEGY_DT_ADAPTATIVE };

class BdfBase
{
    friend class boost::serialization::access;
public:
    typedef std::vector<double>::iterator time_iterator;
    typedef std::vector<double>::const_iterator time_const_iterator;

    BdfBase()
        :
        M_order( 1 ),
        M_name( "bdf" ),
        M_time( 0.0 ),
        M_Ti( 0.0 ),
        M_Tf( 1.0 ),
        M_dt( 1.0 ),
        M_strategy( BDF_STRATEGY_DT_CONSTANT ),
        M_state( BDF_UNITIALIZED ),
        M_n_restart( 0 ),
        M_alpha( BDF_MAX_ORDER ),
        M_beta( BDF_MAX_ORDER )
    {}

    BdfBase( po::variables_map const& vm, std::string name )
        :
        M_order( vm["bdf-time-order"].as<int>() ),
        M_name( name ),
        M_time( vm["bdf-time-initial"].as<double>() ),
        M_Ti( vm["bdf-time-initial"].as<double>() ),
        M_Tf( vm["bdf-time-final"].as<double>() ),
        M_dt( vm["bdf-time-step"].as<double>() ),
        M_strategy( (BDFStragegy)vm["bdf-time-strategy"].as<int>() ),
        M_state( BDF_UNITIALIZED ),
        M_n_restart( 0 ),
        M_alpha( BDF_MAX_ORDER ),
        M_beta( BDF_MAX_ORDER )
    {
        this->init();
    }

    BdfBase( BdfBase const& b )
        :
        M_order( b.M_order ),
        M_name( b.M_name ),
        M_time( b.M_time ),
        M_Ti( b.M_Ti ),
        M_Tf( b.M_Tf ),
        M_dt( b.M_dt ),
        M_strategy( b.M_strategy ),
        M_state( b.M_state ),
        M_n_restart( b.M_n_restart ),
        M_alpha( b.M_alpha ),
        M_beta( b.M_beta )
    {}

    virtual ~BdfBase() {}

    double polyCoefficient( int i ) const
    {
        LIFE_ASSERT( i >=0 && i < BDF_MAX_ORDER-1 ).error( "[BDF] invalid index" );
        return M_beta[this->timeOrder()-1][i]/this->timeStep();
    }
    double polyDerivCoefficient( int i ) const
    {
        LIFE_ASSERT( i >=0 && i < BDF_MAX_ORDER ).error( "[BDF] invalid index" );
        return M_alpha[this->timeOrder()-1][i]/this->timeStep();
    }

    BdfBase& operator=( BdfBase const& b )
    {
        if ( this != &b )
            {
                M_order = b.M_order;
                M_name = b.M_name;
                M_time = b.M_time;
                M_Ti = b.M_Ti;
                M_Tf = b.M_Tf;
                M_dt = b.M_dt;
                M_n_restart = b.M_n_restart;
                M_strategy = b.M_strategy;
                M_state = b.M_state;

                M_alpha = b.M_alpha;
                M_beta = b.M_beta;
            }
        return *this;
    }

    //! save/load Bdf metadata
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version )
    {
        Debug( 5017 ) << "[BDF::serialize] serialize BDFBase\n";
#if 0
        ar & M_order;
        ar & M_name;
        ar & M_time;

        ar & M_n_restart;

        ar & M_Tf;
#endif
        //ar & M_time_orders;
        ar & boost::serialization::make_nvp( "time_values", M_time_values );

        Debug( 5017 ) << "[BDF::serialize] time orders size: " << M_time_orders.size() << "\n";
        Debug( 5017 ) << "[BDF::serialize] time values size: " << M_time_values.size() << "\n";
        for(size_type i = 0; i < M_time_values.size(); ++ i )
            {
                //Log() << "[Bdf] order " << i << "=" << M_time_orders[i] << "\n";
                Log() << "[Bdf::serialize] value " << i << "=" << M_time_values[i] << "\n";
            }
        Debug( 5017 ) << "[BDF::serialize] serialize BDFBase done\n";
    }

    //! return the order in time
    int timeOrder() const { return M_order; }

    //! return the initial time
    double timeInitial() const { return M_Ti; }

    //! return the final time
    double timeFinal() const { return M_Tf; }

    //! return the final step
    double timeStep() const { return M_dt; }

    //! return the BDF strategy
    BDFStragegy strategy() const { return M_strategy; }

    //! return the number of restarts
    int nRestart() const { return M_n_restart; }

    //! return the current time
    double time() const { return M_time; }

    //! return the iteration number
    int iteration() const { return M_iteration; }

    //! return the real time in seconds spent in the iteration
    double realTimePerIteration() const
    {
        LIFE_ASSERT( state() == BDF_RUNNING ).error( "invalid BDF state" );
        M_real_time_per_iteration = M_timer.elapsed();
        return M_real_time_per_iteration;
    }

    //! start the bdf
    double start() const
    {
        M_state = BDF_RUNNING;
        M_timer.restart();
        M_iteration = 1;
        M_time = M_Ti+this->timeStep();
        return M_Ti;
    }

    //! return true if Bdf is finished, false otherwise
    bool isFinished() const
    {
        if ( M_time > M_Tf )
            M_state = BDF_STOPPED;
        return ( M_time > M_Tf );
    }

    /**
     * advance in time
     *
     * \todo implements the strategies here, at the moment constant
     * time step
     */
    double next() const
    {
        LIFE_ASSERT( state() == BDF_RUNNING ).error( "invalid BDF state" );
        M_real_time_per_iteration = M_timer.elapsed();
        M_timer.restart();
        ++M_iteration;
        M_time += M_dt;
        return M_time;
    }

    virtual void shiftRight()
    {
        M_time_values.push_back( this->time() );
    }

    //! return the state of the BDF
    BDFState state() const { return M_state; }

    //! return the relative path where the bdf data is stored
    fs::path path() { return M_path_save; }

    void print() const
    {
        Log() << "============================================================\n";
        Log() << "BDF Information\n";
        Log() << "   time step : " << this->timeStep() << "\n";
        Log() << "time initial : " << this->timeInitial() << "\n";
        Log() << "  time final : " << this->timeFinal() << "\n";
        Log() << "  time order : " << this->timeOrder() << "\n";
    }
protected:
    //! time order
    int M_order;

    //! name of the file holding the bdf data
    std::string M_name;

    //! time
    mutable double M_time;
    mutable int M_iteration;

    //! initial time to start
    double M_Ti;

    //! final time
    double M_Tf;

    //! timestep
    double M_dt;

    //! BDF strategy  (constant timestep or adaptative timestep)
    BDFStragegy M_strategy;

    //! Bdf state
    mutable BDFState M_state;

    int M_n_restart;

    //! timer for real time per iteration
    mutable boost::timer M_timer;

    //! real time spent per iteration (in seconds)
    mutable double M_real_time_per_iteration;

    //! vector that holds the time order at each bdf step
    std::vector<int> M_time_orders;

    //! vector that holds the time value at each bdf step
    std::vector<double> M_time_values;

    //! path to the directory to store the functions
    fs::path M_path_save;

    //! Coefficients \f$ \alpha_i \f$ of the time bdf discretization
    std::vector<ublas::vector<double> > M_alpha;

    //! Coefficients \f$ \beta_i \f$ of the extrapolation
    std::vector<ublas::vector<double> > M_beta;

protected:
    void init()
    {
        for( int i = 0; i < BDF_MAX_ORDER; ++i )
            {
                M_alpha[ i ].resize( i+2 );
                M_beta[ i ].resize( i+1 );
            }

        for ( int i = 0; i < BDF_MAX_ORDER; ++i )
            {
                if (  i == 0 ) // BDF_ORDER_ONE:
                    {
                        M_alpha[i][ 0 ] = 1.; // Backward Euler
                        M_alpha[i][ 1 ] = 1.;
                        M_beta[i][ 0 ] = 1.; // u^{n+1} \approx u^n
                    }
                else if ( i == 1 ) // BDF_ORDER_TWO:
                    {
                        M_alpha[i][ 0 ] = 3. / 2.;
                        M_alpha[i][ 1 ] = 2.;
                        M_alpha[i][ 2 ] = -1. / 2.;
                        M_beta[i][ 0 ] = 2.;
                        M_beta[i][ 1 ] = -1.;
                    }
                else if ( i == 2 ) // BDF_ORDER_THREE:
                    {
                        M_alpha[i][ 0 ] = 11. / 6.;
                        M_alpha[i][ 1 ] = 3.;
                        M_alpha[i][ 2 ] = -3. / 2.;
                        M_alpha[i][ 3 ] = 1. / 3.;
                        M_beta[i][ 0 ] = 3.;
                        M_beta[i][ 1 ] = -3.;
                        M_beta[i][ 2 ] = 1.;
                    }
                else if ( i == 3 ) /// BDF_ORDER_FOUR:
                    {
                        M_alpha[i][ 0 ] = 25. / 12.;
                        M_alpha[i][ 1 ] = 4.;
                        M_alpha[i][ 2 ] = -3.;
                        M_alpha[i][ 3 ] = 4. / 3.;
                        M_alpha[i][ 4 ] = -1. / 4.;
                        M_beta[i][ 0 ] = 4.;
                        M_beta[i][ 1 ] = -6.;
                        M_beta[i][ 2 ] = 4.;
                        M_beta[i][ 3 ] = -1.;
                    }
            }


        std::ostringstream ostr;
        ostr << "bdf_o_" << M_order << "_dt_" << M_dt;
        M_path_save = ostr.str();

        // if directory does not exist, create it
        if ( !fs::exists( M_path_save ) )
            fs::create_directory( M_path_save );

        if ( M_Ti != 0.0 )
            {
                // read the saved bdf data
                if ( fs::exists( M_path_save / "metadata" ) )
                    {
                        Debug( 5017 ) << "[Bdf] loading metadata from " << M_path_save.string() << "\n";

                        fs::ifstream ifs( this->path() / "metadata");


                        boost::archive::text_iarchive ia(ifs);
                        ia >> BOOST_SERIALIZATION_NVP(*this);
                        Debug( 5017 ) << "[Bdf::init()] metadata loaded\n";
                        //BdfBaseMetadata bdfloader( *this );
                        //bdfloader.load();

                        time_iterator it;
                        // look for M_ti in the time values
                        M_iteration = 0;
                        bool found = false;
                        BOOST_FOREACH( double time, M_time_values )
                            {
                                if ( math::abs( time-M_Ti ) < 1e-10 )
                                    {
                                        found = true;
                                        break;
                                    }
                                ++M_iteration;
                            }

                        //it = std::find( M_time_values.begin(), M_time_values.end(), M_Ti );
                        if ( !found )
                            {
                                Debug( 5017 ) << "[Bdf] intial time " << M_Ti << " not found\n";
                                M_Ti = 0.0;
                                M_iteration = 0;
                                M_time_values.resize( 0 );
                                return;
                            }
                        Debug( 5017 ) << "[Bdf] initial time is Ti=" << M_Ti << "\n";

                        Debug( 5017 ) << "[Bdf::init()] file index: " << M_iteration << "\n";
                    }
                else
                    {
                        M_Ti = 0.0;
                        M_time_values.resize( 0 );
                    }
            }

    } // init
};
class BdfBaseMetadata
{
public:
    BdfBaseMetadata( BdfBase& bdf )
        :
        M_bdf( bdf )
    {
    }

    void load()
    {
        fs::ifstream ifs( M_bdf.path() / "metadata");


        boost::archive::text_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(M_bdf);
        Debug( 5017 ) << "[Bdf::init()] metadata loaded\n";
    }

    void save()
    {
        fs::ofstream ofs( M_bdf.path() / "metadata");


        boost::archive::text_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP((BdfBase const&)M_bdf);
        Debug( 5017 ) << "[Bdf::init()] metadata saved\n";
    }

private:
    BdfBase& M_bdf;
};
/**
 * \class Bdf
 * \ingroup SpaceTime
 * \brief Backward differencing formula time discretization
 *
 * A differential equation of the form
 *
 * \f$ M u' = A u + f \f$
 *
 * is discretized in time as
 *
 * \f$ M p'(t_{k+1}) = A u_{k+1} + f_{k+1} \f$
 *
 * where p denotes the polynomial of order n in t that interpolates
 * (t_i,u_i) for i = k-n+1,...,k+1.
 *
 * The approximative time derivative \f$ p'(t_{k+1}) \f$ is a linear
 * combination of state vectors u_i:
 *
 * \f$ p'(t_{k+1}) = \frac{1}{\Delta t} (\alpha_0 u_{k+1} - \sum_{i=0}^n \alpha_i u_{k+1-i} )\f$
 *
 * Thus we have
 *
 * \f$ \frac{\alpha_0}{\Delta t} M u_{k+1} = A u_{k+1} + f + M \bar{p} \f$
 *
 * with
 *
 * \f$ \bar{p} = \frac{1}{\Delta t} \sum_{i=1}^n \alpha_i u_{k+1-i} \f$
 *
 * This class stores the n last state vectors in order to be able to
 * calculate \f$ \bar{p} \f$. It also provides alpha_i
 * and can extrapolate the the new state from the n last states with a
 * polynomial of order n-1:
 *
 * \f$ u_{k+1} \approx \sum_{i=0}^{n-1} \beta_i u_{k-i} \f$
 */
template<typename SpaceType>
class Bdf : public BdfBase
{
    friend class boost::serialization::access;
    typedef BdfBase super;
public:

    typedef SpaceType space_type;
    typedef boost::shared_ptr<space_type>  space_ptrtype;
    typedef typename space_type::element_type element_type;
    typedef typename space_type::return_type return_type;
    typedef typename element_type::value_type value_type;
    //typedef boost::numeric::ublas::vector< element_type > unknowns_type;
    typedef boost::shared_ptr<element_type> unknown_type;
    typedef std::vector< unknown_type > unknowns_type;
    typedef typename node<value_type>::type node_type;

    typedef typename super::time_iterator time_iterator;

    /**
     * Constructor
     *
     * @param space approximation space
     * @param n order of the BDF
     */
    Bdf( po::variables_map const& vm, space_ptrtype const& space, std::string const& name );

    ~Bdf();

    /**
       Initialize all the entries of the unknown vector to be derived with the
       vector u0 (duplicated)
    */
    void initialize( element_type const& u0 );

    /**
       Initialize all the entries of the unknown vector to be derived with a
       set of vectors uv0
    */
    void initialize( unknowns_type const& uv0 );

    /**
       Update the vectors of the previous time steps by shifting on the right
       the old values.
       @param u_curr current (new) value of the state vector
    */
    template<typename container_type>
    void shiftRight( typename space_type::template Element<value_type, container_type> const& u_curr );

    //! Returns the right hand side \f$ \bar{p} \f$ of the time derivative
    //! formula
    element_type polyDeriv() const;

    //! Compute the polynomial extrapolation approximation of order n-1 of
    //! u^{n+1} defined by the n stored state vectors
    element_type poly() const;

    //! Return a vector with the last n state vectors
    unknowns_type const& unknowns() const;

    //! Return a vector with the last n state vectors
    element_type& unknown( int i );

    template<typename container_type>
    void setUnknown( int i,  typename space_type::template Element<value_type, container_type> const& e )
    {
        M_unknowns[i]->assign( e );
    }

    void showMe( std::ostream& __out = std::cout ) const;

private:
    void init();


    void saveCurrent();

    //! save/load Bdf metadata
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version )
    {
        Debug( 5017 ) << "[BDF::serialize] saving/loading archive\n";
        ar & boost::serialization::base_object<BdfBase>(*this);
    }

private:

    //! space
    space_ptrtype M_space;

    //! Last n state vectors
    unknowns_type M_unknowns;
};

template <typename SpaceType>
Bdf<SpaceType>::Bdf( po::variables_map const& vm,
                     space_ptrtype const& __space,
                     std::string const& name  )
    :
    super( vm, name ),
    M_space( __space )
{
    this->init();
}

template <typename SpaceType>
void
Bdf<SpaceType>::init()
{
    super::init();

    M_unknowns.resize( BDF_MAX_ORDER );
    for ( uint8_type __i = 0; __i < ( uint8_type )BDF_MAX_ORDER; ++__i )
        {
            M_unknowns[__i] = unknown_type( new element_type( M_space ) );
            M_unknowns[__i]->zero();
        }

    if ( timeInitial() > 0. )
        {
            for( int p = 0; p < std::max( M_order, M_iteration); ++p )
                {
                    // create and open a character archive for output
                    std::ostringstream ostr;
                    ostr << M_name << "-" << M_iteration-p;

                    Debug( 5017 ) << "[Bdf::init()] load file: " << ostr.str() << "\n";

                    fs::ifstream ifs(M_path_save / ostr.str(), std::ios::binary);


                    // load data from archive
                    boost::archive::binary_iarchive ia(ifs);
                    ia >> *M_unknowns[p];
                }
        }
}


template <typename SpaceType>
Bdf<SpaceType>::~Bdf()
{}


template <typename SpaceType>
void
Bdf<SpaceType>::initialize( element_type const& u0 )
{
    M_time_values.resize( 0 );
    M_time_values.push_back( 0. );
    std::for_each( M_unknowns.begin(), M_unknowns.end(), *boost::lambda::_1 = u0 );
    this->saveCurrent();
}

template <typename SpaceType>
void
Bdf<SpaceType>::initialize( unknowns_type const& uv0 )
{
    M_time_values.resize( 0 );
    M_time_values.push_back( 0. );
    // Check if uv0 has the right dimensions
    //LIFE_ASSERT( uv0.size() == uint16_type(M_order) ).error( "Initial data set are not enough for the selected BDF" );

    std::copy( uv0.begin(), uv0.end(), M_unknowns.begin() );
    this->saveCurrent();
}

template <typename SpaceType>
const
typename Bdf<SpaceType>::unknowns_type&
Bdf<SpaceType>::unknowns() const
{
    return M_unknowns;
}

template <typename SpaceType>
typename Bdf<SpaceType>::element_type&
Bdf<SpaceType>::unknown( int i )
{
    Debug( 5017 ) << "[Bdf::unknown] id: " << i << " l2norm = " << M_unknowns[i]->l2Norm() << "\n";
    return *M_unknowns[i];
}


template <typename SpaceType>
void
Bdf<SpaceType>::saveCurrent()
{
    BdfBaseMetadata bdfsaver( *this );
    bdfsaver.save();

    {
        int dist = M_time_values.size()-1;

        // create and open a character archive for output
        std::ostringstream ostr;
        ostr << M_name << "-" << dist;
        Debug( 5017 ) << "[Bdf::saveCurrent] saving" << ostr.str() << "\n";

        fs::ofstream ofs( M_path_save / ostr.str() );


        // load data from archive
        boost::archive::binary_oarchive oa(ofs);
        oa << *M_unknowns[0];
    }
}
template <typename SpaceType>
template<typename container_type>
void
Bdf<SpaceType>::shiftRight( typename space_type::template Element<value_type, container_type> const& __new_unk )
{
    Debug( 5017 ) << "shiftRight: inserting time " << this->time() << "s\n";
    super::shiftRight();

    // shift all previously stored bdf data
    using namespace boost::lambda;
    typename unknowns_type::reverse_iterator __it = boost::next( M_unknowns.rbegin() );
    std::for_each( M_unknowns.rbegin(), boost::prior( M_unknowns.rend() ),
                   (*lambda::_1 = *(*lambda::var( __it )), ++lambda::var( __it ) ) );
    // u(t^{n}) coefficient is in M_unknowns[0]
    *M_unknowns[0] = __new_unk;
    int i = 0;
    BOOST_FOREACH( boost::shared_ptr<element_type>& t, M_unknowns  )
        {
            Debug( 5017 ) << "[Bdf::shiftright] id: " << i << " l2norm = " << t->l2Norm() << "\n";
            ++i;
        }

    // save newly stored bdf data
    this->saveCurrent();
}


template <typename SpaceType>
typename Bdf<SpaceType>::element_type
Bdf<SpaceType>::polyDeriv() const
{
    element_type __t( M_space );
    __t.zero();

    LIFE_ASSERT( __t.size() == M_space->nDof() )( __t.size() )( M_space->nDof() ).error( "invalid space element size" );
    LIFE_ASSERT( __t.size() == M_unknowns[0]->size() )( __t.size() )( M_unknowns[0]->size() ).error( "invalid space element size" );
    for ( uint8_type i = 0;i < this->timeOrder();++i )
        __t.add( this->polyDerivCoefficient( i+1 ), *M_unknowns[i] );

    return __t;
}

template <typename SpaceType>
typename Bdf<SpaceType>::element_type
Bdf<SpaceType>::poly() const
{
    element_type __t( M_space );
    __t.zero();

    LIFE_ASSERT( __t.size() == M_space->nDof() )( __t.size() )( M_space->nDof() ).error( "invalid space element size" );
    LIFE_ASSERT( __t.size() == M_unknowns[0]->size() )( __t.size() )( M_unknowns[0]->size() ).error( "invalid space element size" );

    for ( uint8_type i = 0;i < this->timeOrder();++i )
        __t.add(  this->polyCoefficient( i ),  *M_unknowns[ i ] );

    return __t;
}

/**
 * command line options
 */
po::options_description bdf_options( std::string const& prefix = "" );

}
#endif
