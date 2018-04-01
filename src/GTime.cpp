
//============================================================================
//
//  This file is part of GFC, the GNSS FOUNDATION CLASS.
//
//  The GFC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GFC is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GFC; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//
//  Copyright 2015, lizhen
//
//============================================================================


#include "GTime.h"

namespace gfc
{
    
    //静态变量的定义及初始化
    //目前存在的问题是：如何保证类的静态变量的线程安全性，可重入性？？？？？
    //
    
    //类TimeSystem相关的初始化
    std::list<GString> gfc::TimeSystem::timesystemTab  = gfc::TimeSystem::Initializer();
    std::list<GString> TimeSystem::Initializer()
    {
        std::list<GString>  tmpTab;
        tmpTab.push_back("tsUKN");
        tmpTab.push_back("tsGPS");
        tmpTab.push_back("tsBDS");
        tmpTab.push_back("tsGLO");
        tmpTab.push_back("tsUTC");
        tmpTab.push_back("tsTAI");
        tmpTab.push_back("tsTT");
        tmpTab.push_back("tsTDB");
        tmpTab.push_back("tsUT1");
        tmpTab.push_back("tsGAL"); // galileo timesystem
        tmpTab.push_back("tsIRN");  // IRNSS time system
        
        return tmpTab;
    }
    
    // register user defined timesystem
    void TimeSystem::RegByName( GString variableName )
    {
        // if variableName does not exist, then register , otherwise throw out exception
        int NumberOfTS(0) ;
        NumberOfTS = static_cast<int>(count(timesystemTab.begin(), timesystemTab.end(), variableName ));
        
        if( NumberOfTS == 0 )
        {
            timesystemTab.push_back(variableName);
        }
        else
        {
            printf("WARNING: TimeSystem : timesystem %s has already existed!\n",variableName.c_str());
        }
    }
    
    void TimeSystem::UnregByName( GString variableName )
    {
        std::list<GString>::iterator myIterator;
        myIterator = find(timesystemTab.begin(),timesystemTab.end(),variableName);
        if( myIterator == timesystemTab.end() )
        {
            printf("WARNING: TimeSystem : timesystem %s hasn't  existed!\n",variableName.c_str());
        }
        else
        {
            timesystemTab.erase(myIterator);
        }
    }
    
    
    
    TimeSystem TimeSystem::GetByName( GString variableName )
    {
        std::list<GString>::iterator myIterator;
        myIterator = find(timesystemTab.begin(),timesystemTab.end(),variableName);
        if( myIterator == timesystemTab.end() )
        {
            // throw out exception, timesystem does not exist
            //非法请求
            TimeSystemUnexist ir("TimeSystem unexist!");
            ir.addLocation(FILE_LOCATION);
            GFC_THROW( ir );
            
        }
        else
        {
            TimeSystem  myts(*myIterator);
            return myts;
        }
    }
    
    
    void TimeSystem::dump( std::ostream& s )
    {
        s<<"ClassName:    "<<"TimeSystem"<<std::endl;
        
        //遍历所有的变量
        std::list<GString>::iterator myit = timesystemTab.begin();
        for(;myit!= timesystemTab.end(); ++myit)
        {
            s<<"TimeSystemName:  "<<*myit<<std::endl;
        }
    }
    
    
    
    
    //类GTime相关的初始化
    std::numeric_limits<double> e;
    double gfc::GTime::eps =1.0E-15;  // e.epsilon();
    
    std::vector< GTime::LeapType  > gfc::GTime::LeapTable = gfc::GTime::InitializeLeapSecTable();
    
    //只要不主动调用该函数(不允许主动调用)，该函数只会被自动调用一次
    std::vector< GTime::LeapType >   GTime::InitializeLeapSecTable()
    {
        // the time in leap seconds table is UTC,
        const  GTime::LeapType myleap[]=
        {
            { 1971,  12,	31,	10 },
            { 1972,  06,	30, 11  },
            { 1972,  12,	31,	12 },
            { 1973,  12,	31,	13 },
            { 1974,  12,	31,	14 },
            { 1975,  12,	31, 15 },
            { 1976,  12,	31,	16 },
            { 1977,  12,	31,	17 },
            { 1978,  12,	31,	18 },
            { 1979,  12,	31,	19 },
            { 1981,  06,	30,	20 },
            { 1982,  06,	30,	21 },
            { 1983,  06,	30,	22 },
            { 1985,  06,	30,	23 },
            { 1987,  12,	31,	24 },
            { 1989,  12,	31,	25 },
            { 1990,  12,	31,	26 },
            { 1992,  06,	30,	27 },
            { 1993,  06,	30,	28 },
            { 1994,  06,	30,	29 },
            { 1995,  12,	31,	30 },
            { 1997,  06,	30,	31 },
            { 1998,  12,	31,	32 },
            { 2005,  12,	31,	33 },
            { 2008,  12,	31,	34 },
            { 2012,  06,	30,	35 },
            { 2015,  7,	1,	36 }
            
        };
        
        int_t  leaptypesize  = sizeof(myleap)/sizeof(LeapType);
        
        std::vector<  GTime::LeapType >  tmpLeap(myleap,myleap+leaptypesize);
        
        //int test = tmpLeap[26].nleap;
        
        return tmpLeap;
    }
    
    //here year month and day should be in UTC
    void  GTime::AddLeapSecond( int year, int month , int day ,int leaps )
    {
        GTime::LeapType  mytype;
        mytype.year = year;
        mytype.month = month;
        mytype.day   = day;
        mytype.nleap = leaps;
        
        //在push_back之前需要检查下该跳秒记录是否有重复
        
        LeapTable.push_back(mytype);
    }
    
    
    /*获取跳秒的函数要是静态的*/
    double GTime::getLeapSecond( int year, int month, int day)
    {
        double leapsec = 0.0;
        if( year < 1972 )  //1972年以前的跳秒不考虑
        {
            InvalidTime e("Class:GTime, year should be larger than 1972.");
            e.addLocation(FILE_LOCATION);
            std::cout<<e<<std::endl;
            GFC_THROW(e);
        }
        // the number of leaps (do not change this)
        
        //int_t NLEAPS = static_cast<int_t>(LeapTable.size()) ;
        
        //LeapType  myleap ;
        for( int i = LeapTable.size()-1; i >= 0; i-- )
        {
            int myyear = LeapTable[i].year;
            int mymonth = LeapTable[i].month;
            int myday = LeapTable[i].day;
            bool t1 = myyear > year;
            bool t2 = ( myyear == year && mymonth > month);
            bool t3 = ( myyear == year && mymonth == month && myday > day );
            if( ( t1||t2||t3) )
            {
                continue;
            }
            //return double(LeapTable[i].nleap);
            leapsec = double( LeapTable[i].nleap);
            break;
        }
        
        return leapsec;
    }
    
    //构造函数
    GTime::GTime()
    {
        m_mjd = 0.0; m_sod = 0.0;
        m_fsod = 0.0;  // unit: nano second
    }
    
    // constructor
    GTime::GTime( long mjd, long sod, double fsod, GString tsStr)
    {
        double NANO = GCONST("NANO");
        TimeSystem ts(tsStr);
        m_ts = ts;
        SetData(ts, mjd, sod, fsod);
    }
    
    
    /*
     拷贝构造函数
     */
    GTime::GTime( const GTime& time )
    {
        this->m_ts = time.m_ts;
        this->m_mjd = time.m_mjd;
        this->m_sod = time.m_sod;
        this->m_fsod = time.m_fsod;
    }
    
    
    TimeSystem GTime::getTimeSystem()
    { return m_ts; }
    
    long       GTime::getMJD() const
    {
        return m_mjd;
    }
    long       GTime::getSOD() const
    {
        return m_sod;
    }
    double     GTime::getFSOD() const
    {
        return m_fsod;
    }

    
    GTime  GTime::J2000()
    {
        //The J2000.0 epoch is precisely Julian date 2451545.0 TT (Terrestrial Time), or January 1, 2000, noon TT. This is equivalent to January 1, 2000, 11:59:27.816 TAI or January 1, 2000, 11:58:55.816 UTC.
        
//        JDTime jdt;
//        jdt.m_jd = 2451545;
//        jdt.m_sod = 0;
//        jdt.m_fsod = 0;
//        jdt.m_ts = gfc::TimeSystem::GetByName("tsTT");
//        GTime J2000T = GTime::JDTime2GTime(jdt);
//        return J2000T;
        
        CivilTime ct(2000,1,1,12,0,0,"tsTT");
        GTime J2000T = GTime::CivilTime2GTime(ct);
        return J2000T;
        
    }

    // the definiton of Beidou System Time
    GTime GTime::BDT0()
    {
        CivilTime cvt;
        cvt.m_year = 2000; cvt.m_month = 1; cvt.m_day =5;
        cvt.m_hour = 12; cvt.m_minute =0; cvt.m_second =0;
        cvt.m_ts = gfc::TimeSystem::GetByName("tsUTC");
        GTime bdt0;
        bdt0.SetFromCivilTime(cvt);
        return bdt0;
    }
    
    // the definiton of GPS System Time
    GTime GTime::GPST0()
    {
        CivilTime cvt;
        cvt.m_year = 1980; cvt.m_month = 1; cvt.m_day =6;
        cvt.m_hour = 0; cvt.m_minute =0; cvt.m_second =0;
        cvt.m_ts = gfc::TimeSystem::GetByName("tsUTC");
        GTime gpst0;
        gpst0.SetFromCivilTime(cvt);
        return gpst0;
    }
    
    
    
    double GTime::TTmTAI() { return 32.184; }
    
    double GTime::TAImGPST() {return 19.0; }
    
    double GTime::TAImUTC( GTime UTC )
    {
        JDTime JD = gfc::GTime::GTime2JDTime(UTC);
        
        CivilTime CVT = gfc::GTime::JDTime2CivilTime(JD);
        
        return gfc::GTime::getLeapSecond(CVT.m_year, CVT.m_month, CVT.m_day);
        
    }
    
    GTime GTime::TAI2TT(GTime TAI)
    {
        double ttmtai = TTmTAI(); // second
        GTime TT = TAI + ttmtai;
        TT.SetTimeSystem(gfc::TimeSystem::GetByName("tsTT"));
        return TT;
    }
    
    GTime GTime::TT2TAI(GTime TT)
    {
        GTime TAI = TT - TTmTAI();
        TAI.SetTimeSystem(gfc::TimeSystem::GetByName("tsTAI"));
        return TAI;
    }
    
    
    GTime GTime::TAI2UTC( GTime TAI )
    {
        double NANO = GCONST("NANO");
        GTime myUTC = TAI; //pretend tai as utc
        myUTC.SetTimeSystem(gfc::TimeSystem::GetByName("tsUTC"));
        double leapsecond = TAImUTC(myUTC);
        double tmp = myUTC.m_sod + myUTC.m_fsod/NANO;
        if( tmp < leapsecond )
        {
            myUTC.m_mjd = myUTC.m_mjd -1;  // yesterday
            leapsecond = TAImUTC(myUTC);
        }
        
        GTime UTC = TAI - leapsecond ;  // ensure the leapsecond is right
        UTC.SetTimeSystem(TimeSystem::GetByName("tsUTC"));
        return UTC;
    }
    
    
    GTime GTime::UTC2TAI( GTime UTC )
    {
        double taimutc = TAImUTC(UTC);
        GTime TAI = UTC + taimutc;
        TAI.SetTimeSystem(gfc::TimeSystem::GetByName("tsTAI"));
        return TAI;
    }
    
    GTime GTime::BDT2GPST(GTime BDT)
    {
        GTime GPST = BDT + 14.0;
        GPST.SetTimeSystem(gfc::TimeSystem::GetByName("tsGPS"));
        return GPST;
    }
    
    GTime GTime::GPST2BDT(GTime GPST)
    {
        GTime BDT = GPST - 14.0;
        BDT.SetTimeSystem(gfc::TimeSystem::GetByName("tsBDS"));
        return BDT;
    }
    
    //transform from GPST to UTC
    GTime GTime::GPST2UTC(GTime GPST)
    {
        GTime UTC ;
        GTime TAI = GPST + TAImGPST();
        TAI.SetTimeSystem(TimeSystem::GetByName("tsTAI"));
        
        //*********************************************************
        //in order to get the TAImUTC, we need to know the UTC time of TAI,
        // just do it in a simple way, we directly change the time system, That may be bugs here!!!!!
        // maybe we need interation !
        // zhen.li  25th, Jan, 2016
        //*********************************************************
        GTime myUTC = TAI;
        myUTC.SetTimeSystem(TimeSystem::GetByName("tsUTC"));
        UTC = TAI - TAImUTC(myUTC);
        UTC.SetTimeSystem(TimeSystem::GetByName("tsUTC"));
        return UTC;
    }
    
    
    
    GTime GTime::UTC2GPST(GTime UTC)
    {
        GTime GPST;
        GTime TAI = UTC + TAImUTC(UTC);
        GPST = TAI - TAImGPST();
        GPST.SetTimeSystem(gfc::TimeSystem::GetByName("tsGPS"));
        return GPST;
    }
    
    // we must get ut1mutc from outside data source.
    // ut1mutc should be in seconds
    GTime GTime::UT12UTC(GTime UT1, double ut1mutc)
    {
        GTime UTC = UT1 - ut1mutc;
        UTC.SetTimeSystem(gfc::TimeSystem::GetByName("tsUTC"));
        return UTC;
    }
    
    GTime GTime::UTC2UT1(GTime UTC, double ut1mutc)
    {
        GTime UT1 = UTC + ut1mutc;
        UT1.SetTimeSystem(gfc::TimeSystem::GetByName("tsUT1"));
        return UT1;
    }
    
    GTime  GTime::TT2TDB(GTime TT,double tdbmtt)
    {
        GTime TDB = TT + tdbmtt;
        TDB.SetTimeSystem(TimeSystem::GetByName("tsTDB"));
        return TDB;
    }
    
    //
    GTime  GTime::TDB2TT(GTime TDB,double tdbmtt)
    {
        GTime TT = TDB - tdbmtt;
        TT.SetTimeSystem(TimeSystem::GetByName("tsTT"));
        return TT;
    }
    
    /*
     *
     * strictly speaking, the parameter GTime should be in TDB,TT can be used instead without materially affecting the results
     * but according to http://www.iausofa.org/sofa_ts_c.pdf , 
     *
     * if providing the full paraments, the accurancy can reach to several nanoseconds,
     * however, if setting the rest parameters to ZEROS, we can still get an accurancy of 2 microseconds.
     *
     *  intputs: TT: the current TT time
     *           ut1mutc: the current ut1mutc;
     *           pos: Geocentric vector of the current position
     * the default value for ut1mutc is 0.0 , and pos is NULL, which will lead to a maximum error of 2 microseconds
     */
    double GTime::TDBmTT( GTime TT ,double ut1mutc  , double* geocentricPOS  )
    {
        double tdbmtt = 0.0;
        
        double date1 =0.0, date2 =0.0;
        double ut = 0.0; // the fraction of one day in ut1
        double elong =0.0;  // the longitude in radians
        double u = 0.0, v =0.0; // the distance from Earth spin axis and the distance from north of equtorial plane, both in kms
        if( geocentricPOS != NULL )
        {
            //calculate the value of elong and u and v
            
        }
        
        //GTime tai = GTime::TT2TAI(TT);
        //GTime utc = GTime::TAI2UTC(tai);
        //GTime ut1 = GTime::UTC2UT1(utc, ut1mutc);
        TimeSystem ts;
        long mjd=0, sod =0;
        double fsod = 0;
       // ut1.GetData(ts, mjd, sod, fsod);
        
        u = (sod + fsod)/86400.0;
        
        TT.GetData(ts, mjd, sod, fsod);
        date1 = 2400000.5;
        date2 = mjd + (sod + fsod)/86400.0;
        
        static const double DJ00 = 2451545.0;
        static const double D2PI = 6.283185307179586476925287;
        static const double DJM  = 365250.0 ; /* Days per Julian millennium */
        static const double DD2R = 1.745329251994329576923691e-2 ;
        
        double t, tsol, w, elsun, emsun, d, elj, els, wt, w0, w1, w2, w3, w4,
        wf, wj;
        int j;
        
        /*
         ** =====================
         ** Fairhead et al. model
         ** =====================
         **
         ** 787 sets of three coefficients.
         **
         ** Each set is
         **    amplitude (microseconds)
         **      frequency (radians per Julian millennium since J2000.0)
         **      phase (radians)
         **
         ** Sets   1-474 are the T**0 terms
         **  "   475-679  "   "  T**1
         **  "   680-764  "   "  T**2
         **  "   765-784  "   "  T**3
         **  "   785-787  "   "  T**4
         */
        
        static const double fairhd[787][3] = {
            /* 1, 10 */
            { 1656.674564e-6,     6283.075849991,  6.240054195 },
            {   22.417471e-6,     5753.384884897,  4.296977442 },
            {   13.839792e-6,    12566.151699983,  6.196904410 },
            {    4.770086e-6,      529.690965095,  0.444401603 },
            {    4.676740e-6,     6069.776754553,  4.021195093 },
            {    2.256707e-6,      213.299095438,  5.543113262 },
            {    1.694205e-6,      -3.523118349,   5.025132748 },
            {    1.554905e-6,    77713.771467920,  5.198467090 },
            {    1.276839e-6,     7860.419392439,  5.988822341 },
            {    1.193379e-6,     5223.693919802,  3.649823730 },
            /* 11, 20 */
            {    1.115322e-6,     3930.209696220,  1.422745069 },
            {    0.794185e-6,    11506.769769794,  2.322313077 },
            {    0.447061e-6,       26.298319800,  3.615796498 },
            {    0.435206e-6,     -398.149003408,  4.349338347 },
            {    0.600309e-6,     1577.343542448,  2.678271909 },
            {    0.496817e-6,     6208.294251424,  5.696701824 },
            {    0.486306e-6,     5884.926846583,  0.520007179 },
            {    0.432392e-6,       74.781598567,  2.435898309 },
            {    0.468597e-6,     6244.942814354,  5.866398759 },
            {    0.375510e-6,     5507.553238667,  4.103476804 },
            /* 21, 30 */
            {    0.243085e-6,     -775.522611324,  3.651837925 },
            {    0.173435e-6,    18849.227549974,  6.153743485 },
            {    0.230685e-6,     5856.477659115,  4.773852582 },
            {    0.203747e-6,    12036.460734888,  4.333987818 },
            {    0.143935e-6,     -796.298006816,  5.957517795 },
            {    0.159080e-6,    10977.078804699,  1.890075226 },
            {    0.119979e-6,       38.133035638,  4.551585768 },
            {    0.118971e-6,     5486.777843175,  1.914547226 },
            {    0.116120e-6,     1059.381930189,  0.873504123 },
            {    0.137927e-6,    11790.629088659,  1.135934669 },
            /* 31, 40 */
            {    0.098358e-6,     2544.314419883,  0.092793886 },
            {    0.101868e-6,    -5573.142801634,  5.984503847 },
            {    0.080164e-6,      206.185548437,  2.095377709 },
            {    0.079645e-6,     4694.002954708,  2.949233637 },
            {    0.062617e-6,       20.775395492,  2.654394814 },
            {    0.075019e-6,     2942.463423292,  4.980931759 },
            {    0.064397e-6,     5746.271337896,  1.280308748 },
            {    0.063814e-6,     5760.498431898,  4.167901731 },
            {    0.048042e-6,     2146.165416475,  1.495846011 },
            {    0.048373e-6,      155.420399434,  2.251573730 },
            /* 41, 50 */
            {    0.058844e-6,      426.598190876,  4.839650148 },
            {    0.046551e-6,       -0.980321068,  0.921573539 },
            {    0.054139e-6,    17260.154654690,  3.411091093 },
            {    0.042411e-6,     6275.962302991,  2.869567043 },
            {    0.040184e-6,       -7.113547001,  3.565975565 },
            {    0.036564e-6,     5088.628839767,  3.324679049 },
            {    0.040759e-6,    12352.852604545,  3.981496998 },
            {    0.036507e-6,      801.820931124,  6.248866009 },
            {    0.036955e-6,     3154.687084896,  5.071801441 },
            {    0.042732e-6,      632.783739313,  5.720622217 },
            /* 51, 60 */
            {    0.042560e-6,   161000.685737473,  1.270837679 },
            {    0.040480e-6,    15720.838784878,  2.546610123 },
            {    0.028244e-6,    -6286.598968340,  5.069663519 },
            {    0.033477e-6,     6062.663207553,  4.144987272 },
            {    0.034867e-6,      522.577418094,  5.210064075 },
            {    0.032438e-6,     6076.890301554,  0.749317412 },
            {    0.030215e-6,     7084.896781115,  3.389610345 },
            {    0.029247e-6,   -71430.695617928,  4.183178762 },
            {    0.033529e-6,     9437.762934887,  2.404714239 },
            {    0.032423e-6,     8827.390269875,  5.541473556 },
            /* 61, 70 */
            {    0.027567e-6,     6279.552731642,  5.040846034 },
            {    0.029862e-6,    12139.553509107,  1.770181024 },
            {    0.022509e-6,    10447.387839604,  1.460726241 },
            {    0.020937e-6,     8429.241266467,  0.652303414 },
            {    0.020322e-6,      419.484643875,  3.735430632 },
            {    0.024816e-6,    -1194.447010225,  1.087136918 },
            {    0.025196e-6,     1748.016413067,  2.901883301 },
            {    0.021691e-6,    14143.495242431,  5.952658009 },
            {    0.017673e-6,     6812.766815086,  3.186129845 },
            {    0.022567e-6,     6133.512652857,  3.307984806 },
            /* 71, 80 */
            {    0.016155e-6,    10213.285546211,  1.331103168 },
            {    0.014751e-6,     1349.867409659,  4.308933301 },
            {    0.015949e-6,     -220.412642439,  4.005298270 },
            {    0.015974e-6,    -2352.866153772,  6.145309371 },
            {    0.014223e-6,    17789.845619785,  2.104551349 },
            {    0.017806e-6,       73.297125859,  3.475975097 },
            {    0.013671e-6,     -536.804512095,  5.971672571 },
            {    0.011942e-6,     8031.092263058,  2.053414715 },
            {    0.014318e-6,    16730.463689596,  3.016058075 },
            {    0.012462e-6,      103.092774219,  1.737438797 },
            /* 81, 90 */
            {    0.010962e-6,        3.590428652,  2.196567739 },
            {    0.015078e-6,    19651.048481098,  3.969480770 },
            {    0.010396e-6,      951.718406251,  5.717799605 },
            {    0.011707e-6,    -4705.732307544,  2.654125618 },
            {    0.010453e-6,     5863.591206116,  1.913704550 },
            {    0.012420e-6,     4690.479836359,  4.734090399 },
            {    0.011847e-6,     5643.178563677,  5.489005403 },
            {    0.008610e-6,     3340.612426700,  3.661698944 },
            {    0.011622e-6,     5120.601145584,  4.863931876 },
            {    0.010825e-6,      553.569402842,  0.842715011 },
            /* 91, 100 */
            {    0.008666e-6,     -135.065080035,  3.293406547 },
            {    0.009963e-6,      149.563197135,  4.870690598 },
            {    0.009858e-6,     6309.374169791,  1.061816410 },
            {    0.007959e-6,      316.391869657,  2.465042647 },
            {    0.010099e-6,      283.859318865,  1.942176992 },
            {    0.007147e-6,     -242.728603974,  3.661486981 },
            {    0.007505e-6,     5230.807466803,  4.920937029 },
            {    0.008323e-6,    11769.853693166,  1.229392026 },
            {    0.007490e-6,    -6256.777530192,  3.658444681 },
            {    0.009370e-6,   149854.400134205,  0.673880395 },
            /* 101, 110 */
            {    0.007117e-6,       38.027672636,  5.294249518 },
            {    0.007857e-6,    12168.002696575,  0.525733528 },
            {    0.007019e-6,     6206.809778716,  0.837688810 },
            {    0.006056e-6,      955.599741609,  4.194535082 },
            {    0.008107e-6,    13367.972631107,  3.793235253 },
            {    0.006731e-6,     5650.292110678,  5.639906583 },
            {    0.007332e-6,       36.648562930,  0.114858677 },
            {    0.006366e-6,     4164.311989613,  2.262081818 },
            {    0.006858e-6,     5216.580372801,  0.642063318 },
            {    0.006919e-6,     6681.224853400,  6.018501522 },
            /* 111, 120 */
            {    0.006826e-6,     7632.943259650,  3.458654112 },
            {    0.005308e-6,    -1592.596013633,  2.500382359 },
            {    0.005096e-6,    11371.704689758,  2.547107806 },
            {    0.004841e-6,     5333.900241022,  0.437078094 },
            {    0.005582e-6,     5966.683980335,  2.246174308 },
            {    0.006304e-6,    11926.254413669,  2.512929171 },
            {    0.006603e-6,    23581.258177318,  5.393136889 },
            {    0.005123e-6,       -1.484472708,  2.999641028 },
            {    0.004648e-6,     1589.072895284,  1.275847090 },
            {    0.005119e-6,     6438.496249426,  1.486539246 },
            /* 121, 130 */
            {    0.004521e-6,     4292.330832950,  6.140635794 },
            {    0.005680e-6,    23013.539539587,  4.557814849 },
            {    0.005488e-6,       -3.455808046,  0.090675389 },
            {    0.004193e-6,     7234.794256242,  4.869091389 },
            {    0.003742e-6,     7238.675591600,  4.691976180 },
            {    0.004148e-6,     -110.206321219,  3.016173439 },
            {    0.004553e-6,    11499.656222793,  5.554998314 },
            {    0.004892e-6,     5436.993015240,  1.475415597 },
            {    0.004044e-6,     4732.030627343,  1.398784824 },
            {    0.004164e-6,    12491.370101415,  5.650931916 },
            /* 131, 140 */
            {    0.004349e-6,    11513.883316794,  2.181745369 },
            {    0.003919e-6,    12528.018664345,  5.823319737 },
            {    0.003129e-6,     6836.645252834,  0.003844094 },
            {    0.004080e-6,    -7058.598461315,  3.690360123 },
            {    0.003270e-6,       76.266071276,  1.517189902 },
            {    0.002954e-6,     6283.143160294,  4.447203799 },
            {    0.002872e-6,       28.449187468,  1.158692983 },
            {    0.002881e-6,      735.876513532,  0.349250250 },
            {    0.003279e-6,     5849.364112115,  4.893384368 },
            {    0.003625e-6,     6209.778724132,  1.473760578 },
            /* 141, 150 */
            {    0.003074e-6,      949.175608970,  5.185878737 },
            {    0.002775e-6,     9917.696874510,  1.030026325 },
            {    0.002646e-6,    10973.555686350,  3.918259169 },
            {    0.002575e-6,    25132.303399966,  6.109659023 },
            {    0.003500e-6,      263.083923373,  1.892100742 },
            {    0.002740e-6,    18319.536584880,  4.320519510 },
            {    0.002464e-6,      202.253395174,  4.698203059 },
            {    0.002409e-6,        2.542797281,  5.325009315 },
            {    0.003354e-6,   -90955.551694697,  1.942656623 },
            {    0.002296e-6,     6496.374945429,  5.061810696 },
            /* 151, 160 */
            {    0.003002e-6,     6172.869528772,  2.797822767 },
            {    0.003202e-6,    27511.467873537,  0.531673101 },
            {    0.002954e-6,    -6283.008539689,  4.533471191 },
            {    0.002353e-6,      639.897286314,  3.734548088 },
            {    0.002401e-6,    16200.772724501,  2.605547070 },
            {    0.003053e-6,   233141.314403759,  3.029030662 },
            {    0.003024e-6,    83286.914269554,  2.355556099 },
            {    0.002863e-6,    17298.182327326,  5.240963796 },
            {    0.002103e-6,    -7079.373856808,  5.756641637 },
            {    0.002303e-6,    83996.847317911,  2.013686814 },
            /* 161, 170 */
            {    0.002303e-6,    18073.704938650,  1.089100410 },
            {    0.002381e-6,       63.735898303,  0.759188178 },
            {    0.002493e-6,     6386.168624210,  0.645026535 },
            {    0.002366e-6,        3.932153263,  6.215885448 },
            {    0.002169e-6,    11015.106477335,  4.845297676 },
            {    0.002397e-6,     6243.458341645,  3.809290043 },
            {    0.002183e-6,     1162.474704408,  6.179611691 },
            {    0.002353e-6,     6246.427287062,  4.781719760 },
            {    0.002199e-6,     -245.831646229,  5.956152284 },
            {    0.001729e-6,     3894.181829542,  1.264976635 },
            /* 171, 180 */
            {    0.001896e-6,    -3128.388765096,  4.914231596 },
            {    0.002085e-6,       35.164090221,  1.405158503 },
            {    0.002024e-6,    14712.317116458,  2.752035928 },
            {    0.001737e-6,     6290.189396992,  5.280820144 },
            {    0.002229e-6,      491.557929457,  1.571007057 },
            {    0.001602e-6,    14314.168113050,  4.203664806 },
            {    0.002186e-6,      454.909366527,  1.402101526 },
            {    0.001897e-6,    22483.848574493,  4.167932508 },
            {    0.001825e-6,    -3738.761430108,  0.545828785 },
            {    0.001894e-6,     1052.268383188,  5.817167450 },
            /* 181, 190 */
            {    0.001421e-6,       20.355319399,  2.419886601 },
            {    0.001408e-6,    10984.192351700,  2.732084787 },
            {    0.001847e-6,    10873.986030480,  2.903477885 },
            {    0.001391e-6,    -8635.942003763,  0.593891500 },
            {    0.001388e-6,       -7.046236698,  1.166145902 },
            {    0.001810e-6,   -88860.057071188,  0.487355242 },
            {    0.001288e-6,    -1990.745017041,  3.913022880 },
            {    0.001297e-6,    23543.230504682,  3.063805171 },
            {    0.001335e-6,     -266.607041722,  3.995764039 },
            {    0.001376e-6,    10969.965257698,  5.152914309 },
            /* 191, 200 */
            {    0.001745e-6,   244287.600007027,  3.626395673 },
            {    0.001649e-6,    31441.677569757,  1.952049260 },
            {    0.001416e-6,     9225.539273283,  4.996408389 },
            {    0.001238e-6,     4804.209275927,  5.503379738 },
            {    0.001472e-6,     4590.910180489,  4.164913291 },
            {    0.001169e-6,     6040.347246017,  5.841719038 },
            {    0.001039e-6,     5540.085789459,  2.769753519 },
            {    0.001004e-6,     -170.672870619,  0.755008103 },
            {    0.001284e-6,    10575.406682942,  5.306538209 },
            {    0.001278e-6,       71.812653151,  4.713486491 },
            /* 201, 210 */
            {    0.001321e-6,    18209.330263660,  2.624866359 },
            {    0.001297e-6,    21228.392023546,  0.382603541 },
            {    0.000954e-6,     6282.095528923,  0.882213514 },
            {    0.001145e-6,     6058.731054289,  1.169483931 },
            {    0.000979e-6,     5547.199336460,  5.448375984 },
            {    0.000987e-6,    -6262.300454499,  2.656486959 },
            {    0.001070e-6,  -154717.609887482,  1.827624012 },
            {    0.000991e-6,     4701.116501708,  4.387001801 },
            {    0.001155e-6,      -14.227094002,  3.042700750 },
            {    0.001176e-6,      277.034993741,  3.335519004 },
            /* 211, 220 */
            {    0.000890e-6,    13916.019109642,  5.601498297 },
            {    0.000884e-6,    -1551.045222648,  1.088831705 },
            {    0.000876e-6,     5017.508371365,  3.969902609 },
            {    0.000806e-6,    15110.466119866,  5.142876744 },
            {    0.000773e-6,    -4136.910433516,  0.022067765 },
            {    0.001077e-6,      175.166059800,  1.844913056 },
            {    0.000954e-6,    -6284.056171060,  0.968480906 },
            {    0.000737e-6,     5326.786694021,  4.923831588 },
            {    0.000845e-6,     -433.711737877,  4.749245231 },
            {    0.000819e-6,     8662.240323563,  5.991247817 },
            /* 221, 230 */
            {    0.000852e-6,      199.072001436,  2.189604979 },
            {    0.000723e-6,    17256.631536341,  6.068719637 },
            {    0.000940e-6,     6037.244203762,  6.197428148 },
            {    0.000885e-6,    11712.955318231,  3.280414875 },
            {    0.000706e-6,    12559.038152982,  2.824848947 },
            {    0.000732e-6,     2379.164473572,  2.501813417 },
            {    0.000764e-6,    -6127.655450557,  2.236346329 },
            {    0.000908e-6,      131.541961686,  2.521257490 },
            {    0.000907e-6,    35371.887265976,  3.370195967 },
            {    0.000673e-6,     1066.495477190,  3.876512374 },
            /* 231, 240 */
            {    0.000814e-6,    17654.780539750,  4.627122566 },
            {    0.000630e-6,       36.027866677,  0.156368499 },
            {    0.000798e-6,      515.463871093,  5.151962502 },
            {    0.000798e-6,      148.078724426,  5.909225055 },
            {    0.000806e-6,      309.278322656,  6.054064447 },
            {    0.000607e-6,      -39.617508346,  2.839021623 },
            {    0.000601e-6,      412.371096874,  3.984225404 },
            {    0.000646e-6,    11403.676995575,  3.852959484 },
            {    0.000704e-6,    13521.751441591,  2.300991267 },
            {    0.000603e-6,   -65147.619767937,  4.140083146 },
            /* 241, 250 */
            {    0.000609e-6,    10177.257679534,  0.437122327 },
            {    0.000631e-6,     5767.611978898,  4.026532329 },
            {    0.000576e-6,    11087.285125918,  4.760293101 },
            {    0.000674e-6,    14945.316173554,  6.270510511 },
            {    0.000726e-6,     5429.879468239,  6.039606892 },
            {    0.000710e-6,    28766.924424484,  5.672617711 },
            {    0.000647e-6,    11856.218651625,  3.397132627 },
            {    0.000678e-6,    -5481.254918868,  6.249666675 },
            {    0.000618e-6,    22003.914634870,  2.466427018 },
            {    0.000738e-6,     6134.997125565,  2.242668890 },
            /* 251, 260 */
            {    0.000660e-6,      625.670192312,  5.864091907 },
            {    0.000694e-6,     3496.032826134,  2.668309141 },
            {    0.000531e-6,     6489.261398429,  1.681888780 },
            {    0.000611e-6,  -143571.324284214,  2.424978312 },
            {    0.000575e-6,    12043.574281889,  4.216492400 },
            {    0.000553e-6,    12416.588502848,  4.772158039 },
            {    0.000689e-6,     4686.889407707,  6.224271088 },
            {    0.000495e-6,     7342.457780181,  3.817285811 },
            {    0.000567e-6,     3634.621024518,  1.649264690 },
            {    0.000515e-6,    18635.928454536,  3.945345892 },
            /* 261, 270 */
            {    0.000486e-6,     -323.505416657,  4.061673868 },
            {    0.000662e-6,    25158.601719765,  1.794058369 },
            {    0.000509e-6,      846.082834751,  3.053874588 },
            {    0.000472e-6,   -12569.674818332,  5.112133338 },
            {    0.000461e-6,     6179.983075773,  0.513669325 },
            {    0.000641e-6,    83467.156352816,  3.210727723 },
            {    0.000520e-6,    10344.295065386,  2.445597761 },
            {    0.000493e-6,    18422.629359098,  1.676939306 },
            {    0.000478e-6,     1265.567478626,  5.487314569 },
            {    0.000472e-6,      -18.159247265,  1.999707589 },
            /* 271, 280 */
            {    0.000559e-6,    11190.377900137,  5.783236356 },
            {    0.000494e-6,     9623.688276691,  3.022645053 },
            {    0.000463e-6,     5739.157790895,  1.411223013 },
            {    0.000432e-6,    16858.482532933,  1.179256434 },
            {    0.000574e-6,    72140.628666286,  1.758191830 },
            {    0.000484e-6,    17267.268201691,  3.290589143 },
            {    0.000550e-6,     4907.302050146,  0.864024298 },
            {    0.000399e-6,       14.977853527,  2.094441910 },
            {    0.000491e-6,      224.344795702,  0.878372791 },
            {    0.000432e-6,    20426.571092422,  6.003829241 },
            /* 281, 290 */
            {    0.000481e-6,     5749.452731634,  4.309591964 },
            {    0.000480e-6,     5757.317038160,  1.142348571 },
            {    0.000485e-6,     6702.560493867,  0.210580917 },
            {    0.000426e-6,     6055.549660552,  4.274476529 },
            {    0.000480e-6,     5959.570433334,  5.031351030 },
            {    0.000466e-6,    12562.628581634,  4.959581597 },
            {    0.000520e-6,    39302.096962196,  4.788002889 },
            {    0.000458e-6,    12132.439962106,  1.880103788 },
            {    0.000470e-6,    12029.347187887,  1.405611197 },
            {    0.000416e-6,    -7477.522860216,  1.082356330 },
            /* 291, 300 */
            {    0.000449e-6,    11609.862544012,  4.179989585 },
            {    0.000465e-6,    17253.041107690,  0.353496295 },
            {    0.000362e-6,    -4535.059436924,  1.583849576 },
            {    0.000383e-6,    21954.157609398,  3.747376371 },
            {    0.000389e-6,       17.252277143,  1.395753179 },
            {    0.000331e-6,    18052.929543158,  0.566790582 },
            {    0.000430e-6,    13517.870106233,  0.685827538 },
            {    0.000368e-6,    -5756.908003246,  0.731374317 },
            {    0.000330e-6,    10557.594160824,  3.710043680 },
            {    0.000332e-6,    20199.094959633,  1.652901407 },
            /* 301, 310 */
            {    0.000384e-6,    11933.367960670,  5.827781531 },
            {    0.000387e-6,    10454.501386605,  2.541182564 },
            {    0.000325e-6,    15671.081759407,  2.178850542 },
            {    0.000318e-6,      138.517496871,  2.253253037 },
            {    0.000305e-6,     9388.005909415,  0.578340206 },
            {    0.000352e-6,     5749.861766548,  3.000297967 },
            {    0.000311e-6,     6915.859589305,  1.693574249 },
            {    0.000297e-6,    24072.921469776,  1.997249392 },
            {    0.000363e-6,     -640.877607382,  5.071820966 },
            {    0.000323e-6,    12592.450019783,  1.072262823 },
            /* 311, 320 */
            {    0.000341e-6,    12146.667056108,  4.700657997 },
            {    0.000290e-6,     9779.108676125,  1.812320441 },
            {    0.000342e-6,     6132.028180148,  4.322238614 },
            {    0.000329e-6,     6268.848755990,  3.033827743 },
            {    0.000374e-6,    17996.031168222,  3.388716544 },
            {    0.000285e-6,     -533.214083444,  4.687313233 },
            {    0.000338e-6,     6065.844601290,  0.877776108 },
            {    0.000276e-6,       24.298513841,  0.770299429 },
            {    0.000336e-6,    -2388.894020449,  5.353796034 },
            {    0.000290e-6,     3097.883822726,  4.075291557 },
            /* 321, 330 */
            {    0.000318e-6,      709.933048357,  5.941207518 },
            {    0.000271e-6,    13095.842665077,  3.208912203 },
            {    0.000331e-6,     6073.708907816,  4.007881169 },
            {    0.000292e-6,      742.990060533,  2.714333592 },
            {    0.000362e-6,    29088.811415985,  3.215977013 },
            {    0.000280e-6,    12359.966151546,  0.710872502 },
            {    0.000267e-6,    10440.274292604,  4.730108488 },
            {    0.000262e-6,      838.969287750,  1.327720272 },
            {    0.000250e-6,    16496.361396202,  0.898769761 },
            {    0.000325e-6,    20597.243963041,  0.180044365 },
            /* 331, 340 */
            {    0.000268e-6,     6148.010769956,  5.152666276 },
            {    0.000284e-6,     5636.065016677,  5.655385808 },
            {    0.000301e-6,     6080.822454817,  2.135396205 },
            {    0.000294e-6,     -377.373607916,  3.708784168 },
            {    0.000236e-6,     2118.763860378,  1.733578756 },
            {    0.000234e-6,     5867.523359379,  5.575209112 },
            {    0.000268e-6,  -226858.238553767,  0.069432392 },
            {    0.000265e-6,   167283.761587465,  4.369302826 },
            {    0.000280e-6,    28237.233459389,  5.304829118 },
            {    0.000292e-6,    12345.739057544,  4.096094132 },
            /* 341, 350 */
            {    0.000223e-6,    19800.945956225,  3.069327406 },
            {    0.000301e-6,    43232.306658416,  6.205311188 },
            {    0.000264e-6,    18875.525869774,  1.417263408 },
            {    0.000304e-6,    -1823.175188677,  3.409035232 },
            {    0.000301e-6,      109.945688789,  0.510922054 },
            {    0.000260e-6,      813.550283960,  2.389438934 },
            {    0.000299e-6,   316428.228673312,  5.384595078 },
            {    0.000211e-6,     5756.566278634,  3.789392838 },
            {    0.000209e-6,     5750.203491159,  1.661943545 },
            {    0.000240e-6,    12489.885628707,  5.684549045 },
            /* 351, 360 */
            {    0.000216e-6,     6303.851245484,  3.862942261 },
            {    0.000203e-6,     1581.959348283,  5.549853589 },
            {    0.000200e-6,     5642.198242609,  1.016115785 },
            {    0.000197e-6,      -70.849445304,  4.690702525 },
            {    0.000227e-6,     6287.008003254,  2.911891613 },
            {    0.000197e-6,      533.623118358,  1.048982898 },
            {    0.000205e-6,    -6279.485421340,  1.829362730 },
            {    0.000209e-6,   -10988.808157535,  2.636140084 },
            {    0.000208e-6,     -227.526189440,  4.127883842 },
            {    0.000191e-6,      415.552490612,  4.401165650 },
            /* 361, 370 */
            {    0.000190e-6,    29296.615389579,  4.175658539 },
            {    0.000264e-6,    66567.485864652,  4.601102551 },
            {    0.000256e-6,    -3646.350377354,  0.506364778 },
            {    0.000188e-6,    13119.721102825,  2.032195842 },
            {    0.000185e-6,     -209.366942175,  4.694756586 },
            {    0.000198e-6,    25934.124331089,  3.832703118 },
            {    0.000195e-6,     4061.219215394,  3.308463427 },
            {    0.000234e-6,     5113.487598583,  1.716090661 },
            {    0.000188e-6,     1478.866574064,  5.686865780 },
            {    0.000222e-6,    11823.161639450,  1.942386641 },
            /* 371, 380 */
            {    0.000181e-6,    10770.893256262,  1.999482059 },
            {    0.000171e-6,     6546.159773364,  1.182807992 },
            {    0.000206e-6,       70.328180442,  5.934076062 },
            {    0.000169e-6,    20995.392966449,  2.169080622 },
            {    0.000191e-6,    10660.686935042,  5.405515999 },
            {    0.000228e-6,    33019.021112205,  4.656985514 },
            {    0.000184e-6,    -4933.208440333,  3.327476868 },
            {    0.000220e-6,     -135.625325010,  1.765430262 },
            {    0.000166e-6,    23141.558382925,  3.454132746 },
            {    0.000191e-6,     6144.558353121,  5.020393445 },
            /* 381, 390 */
            {    0.000180e-6,     6084.003848555,  0.602182191 },
            {    0.000163e-6,    17782.732072784,  4.960593133 },
            {    0.000225e-6,    16460.333529525,  2.596451817 },
            {    0.000222e-6,     5905.702242076,  3.731990323 },
            {    0.000204e-6,      227.476132789,  5.636192701 },
            {    0.000159e-6,    16737.577236597,  3.600691544 },
            {    0.000200e-6,     6805.653268085,  0.868220961 },
            {    0.000187e-6,    11919.140866668,  2.629456641 },
            {    0.000161e-6,      127.471796607,  2.862574720 },
            {    0.000205e-6,     6286.666278643,  1.742882331 },
            /* 391, 400 */
            {    0.000189e-6,      153.778810485,  4.812372643 },
            {    0.000168e-6,    16723.350142595,  0.027860588 },
            {    0.000149e-6,    11720.068865232,  0.659721876 },
            {    0.000189e-6,     5237.921013804,  5.245313000 },
            {    0.000143e-6,     6709.674040867,  4.317625647 },
            {    0.000146e-6,     4487.817406270,  4.815297007 },
            {    0.000144e-6,     -664.756045130,  5.381366880 },
            {    0.000175e-6,     5127.714692584,  4.728443327 },
            {    0.000162e-6,     6254.626662524,  1.435132069 },
            {    0.000187e-6,    47162.516354635,  1.354371923 },
            /* 401, 410 */
            {    0.000146e-6,    11080.171578918,  3.369695406 },
            {    0.000180e-6,     -348.924420448,  2.490902145 },
            {    0.000148e-6,      151.047669843,  3.799109588 },
            {    0.000157e-6,     6197.248551160,  1.284375887 },
            {    0.000167e-6,      146.594251718,  0.759969109 },
            {    0.000133e-6,    -5331.357443741,  5.409701889 },
            {    0.000154e-6,       95.979227218,  3.366890614 },
            {    0.000148e-6,    -6418.140930027,  3.384104996 },
            {    0.000128e-6,    -6525.804453965,  3.803419985 },
            {    0.000130e-6,    11293.470674356,  0.939039445 },
            /* 411, 420 */
            {    0.000152e-6,    -5729.506447149,  0.734117523 },
            {    0.000138e-6,      210.117701700,  2.564216078 },
            {    0.000123e-6,     6066.595360816,  4.517099537 },
            {    0.000140e-6,    18451.078546566,  0.642049130 },
            {    0.000126e-6,    11300.584221356,  3.485280663 },
            {    0.000119e-6,    10027.903195729,  3.217431161 },
            {    0.000151e-6,     4274.518310832,  4.404359108 },
            {    0.000117e-6,     6072.958148291,  0.366324650 },
            {    0.000165e-6,    -7668.637425143,  4.298212528 },
            {    0.000117e-6,    -6245.048177356,  5.379518958 },
            /* 421, 430 */
            {    0.000130e-6,    -5888.449964932,  4.527681115 },
            {    0.000121e-6,     -543.918059096,  6.109429504 },
            {    0.000162e-6,     9683.594581116,  5.720092446 },
            {    0.000141e-6,     6219.339951688,  0.679068671 },
            {    0.000118e-6,    22743.409379516,  4.881123092 },
            {    0.000129e-6,     1692.165669502,  0.351407289 },
            {    0.000126e-6,     5657.405657679,  5.146592349 },
            {    0.000114e-6,      728.762966531,  0.520791814 },
            {    0.000120e-6,       52.596639600,  0.948516300 },
            {    0.000115e-6,       65.220371012,  3.504914846 },
            /* 431, 440 */
            {    0.000126e-6,     5881.403728234,  5.577502482 },
            {    0.000158e-6,   163096.180360983,  2.957128968 },
            {    0.000134e-6,    12341.806904281,  2.598576764 },
            {    0.000151e-6,    16627.370915377,  3.985702050 },
            {    0.000109e-6,     1368.660252845,  0.014730471 },
            {    0.000131e-6,     6211.263196841,  0.085077024 },
            {    0.000146e-6,     5792.741760812,  0.708426604 },
            {    0.000146e-6,      -77.750543984,  3.121576600 },
            {    0.000107e-6,     5341.013788022,  0.288231904 },
            {    0.000138e-6,     6281.591377283,  2.797450317 },
            /* 441, 450 */
            {    0.000113e-6,    -6277.552925684,  2.788904128 },
            {    0.000115e-6,     -525.758811831,  5.895222200 },
            {    0.000138e-6,     6016.468808270,  6.096188999 },
            {    0.000139e-6,    23539.707386333,  2.028195445 },
            {    0.000146e-6,    -4176.041342449,  4.660008502 },
            {    0.000107e-6,    16062.184526117,  4.066520001 },
            {    0.000142e-6,    83783.548222473,  2.936315115 },
            {    0.000128e-6,     9380.959672717,  3.223844306 },
            {    0.000135e-6,     6205.325306007,  1.638054048 },
            {    0.000101e-6,     2699.734819318,  5.481603249 },
            /* 451, 460 */
            {    0.000104e-6,     -568.821874027,  2.205734493 },
            {    0.000103e-6,     6321.103522627,  2.440421099 },
            {    0.000119e-6,     6321.208885629,  2.547496264 },
            {    0.000138e-6,     1975.492545856,  2.314608466 },
            {    0.000121e-6,      137.033024162,  4.539108237 },
            {    0.000123e-6,    19402.796952817,  4.538074405 },
            {    0.000119e-6,    22805.735565994,  2.869040566 },
            {    0.000133e-6,    64471.991241142,  6.056405489 },
            {    0.000129e-6,      -85.827298831,  2.540635083 },
            {    0.000131e-6,    13613.804277336,  4.005732868 },
            /* 461, 470 */
            {    0.000104e-6,     9814.604100291,  1.959967212 },
            {    0.000112e-6,    16097.679950283,  3.589026260 },
            {    0.000123e-6,     2107.034507542,  1.728627253 },
            {    0.000121e-6,    36949.230808424,  6.072332087 },
            {    0.000108e-6,   -12539.853380183,  3.716133846 },
            {    0.000113e-6,    -7875.671863624,  2.725771122 },
            {    0.000109e-6,     4171.425536614,  4.033338079 },
            {    0.000101e-6,     6247.911759770,  3.441347021 },
            {    0.000113e-6,     7330.728427345,  0.656372122 },
            {    0.000113e-6,    51092.726050855,  2.791483066 },
            /* 471, 480 */
            {    0.000106e-6,     5621.842923210,  1.815323326 },
            {    0.000101e-6,      111.430161497,  5.711033677 },
            {    0.000103e-6,      909.818733055,  2.812745443 },
            {    0.000101e-6,     1790.642637886,  1.965746028 },
            
            /* T */
            {  102.156724e-6,     6283.075849991,  4.249032005 },
            {    1.706807e-6,    12566.151699983,  4.205904248 },
            {    0.269668e-6,      213.299095438,  3.400290479 },
            {    0.265919e-6,      529.690965095,  5.836047367 },
            {    0.210568e-6,       -3.523118349,  6.262738348 },
            {    0.077996e-6,     5223.693919802,  4.670344204 },
            /* 481, 490 */
            {    0.054764e-6,     1577.343542448,  4.534800170 },
            {    0.059146e-6,       26.298319800,  1.083044735 },
            {    0.034420e-6,     -398.149003408,  5.980077351 },
            {    0.032088e-6,    18849.227549974,  4.162913471 },
            {    0.033595e-6,     5507.553238667,  5.980162321 },
            {    0.029198e-6,     5856.477659115,  0.623811863 },
            {    0.027764e-6,      155.420399434,  3.745318113 },
            {    0.025190e-6,     5746.271337896,  2.980330535 },
            {    0.022997e-6,     -796.298006816,  1.174411803 },
            {    0.024976e-6,     5760.498431898,  2.467913690 },
            /* 491, 500 */
            {    0.021774e-6,      206.185548437,  3.854787540 },
            {    0.017925e-6,     -775.522611324,  1.092065955 },
            {    0.013794e-6,      426.598190876,  2.699831988 },
            {    0.013276e-6,     6062.663207553,  5.845801920 },
            {    0.011774e-6,    12036.460734888,  2.292832062 },
            {    0.012869e-6,     6076.890301554,  5.333425680 },
            {    0.012152e-6,     1059.381930189,  6.222874454 },
            {    0.011081e-6,       -7.113547001,  5.154724984 },
            {    0.010143e-6,     4694.002954708,  4.044013795 },
            {    0.009357e-6,     5486.777843175,  3.416081409 },
            /* 501, 510 */
            {    0.010084e-6,      522.577418094,  0.749320262 },
            {    0.008587e-6,    10977.078804699,  2.777152598 },
            {    0.008628e-6,     6275.962302991,  4.562060226 },
            {    0.008158e-6,     -220.412642439,  5.806891533 },
            {    0.007746e-6,     2544.314419883,  1.603197066 },
            {    0.007670e-6,     2146.165416475,  3.000200440 },
            {    0.007098e-6,       74.781598567,  0.443725817 },
            {    0.006180e-6,     -536.804512095,  1.302642751 },
            {    0.005818e-6,     5088.628839767,  4.827723531 },
            {    0.004945e-6,    -6286.598968340,  0.268305170 },
            /* 511, 520 */
            {    0.004774e-6,     1349.867409659,  5.808636673 },
            {    0.004687e-6,     -242.728603974,  5.154890570 },
            {    0.006089e-6,     1748.016413067,  4.403765209 },
            {    0.005975e-6,    -1194.447010225,  2.583472591 },
            {    0.004229e-6,      951.718406251,  0.931172179 },
            {    0.005264e-6,      553.569402842,  2.336107252 },
            {    0.003049e-6,     5643.178563677,  1.362634430 },
            {    0.002974e-6,     6812.766815086,  1.583012668 },
            {    0.003403e-6,    -2352.866153772,  2.552189886 },
            {    0.003030e-6,      419.484643875,  5.286473844 },
            /* 521, 530 */
            {    0.003210e-6,       -7.046236698,  1.863796539 },
            {    0.003058e-6,     9437.762934887,  4.226420633 },
            {    0.002589e-6,    12352.852604545,  1.991935820 },
            {    0.002927e-6,     5216.580372801,  2.319951253 },
            {    0.002425e-6,     5230.807466803,  3.084752833 },
            {    0.002656e-6,     3154.687084896,  2.487447866 },
            {    0.002445e-6,    10447.387839604,  2.347139160 },
            {    0.002990e-6,     4690.479836359,  6.235872050 },
            {    0.002890e-6,     5863.591206116,  0.095197563 },
            {    0.002498e-6,     6438.496249426,  2.994779800 },
            /* 531, 540 */
            {    0.001889e-6,     8031.092263058,  3.569003717 },
            {    0.002567e-6,      801.820931124,  3.425611498 },
            {    0.001803e-6,   -71430.695617928,  2.192295512 },
            {    0.001782e-6,        3.932153263,  5.180433689 },
            {    0.001694e-6,    -4705.732307544,  4.641779174 },
            {    0.001704e-6,    -1592.596013633,  3.997097652 },
            {    0.001735e-6,     5849.364112115,  0.417558428 },
            {    0.001643e-6,     8429.241266467,  2.180619584 },
            {    0.001680e-6,       38.133035638,  4.164529426 },
            {    0.002045e-6,     7084.896781115,  0.526323854 },
            /* 541, 550 */
            {    0.001458e-6,     4292.330832950,  1.356098141 },
            {    0.001437e-6,       20.355319399,  3.895439360 },
            {    0.001738e-6,     6279.552731642,  0.087484036 },
            {    0.001367e-6,    14143.495242431,  3.987576591 },
            {    0.001344e-6,     7234.794256242,  0.090454338 },
            {    0.001438e-6,    11499.656222793,  0.974387904 },
            {    0.001257e-6,     6836.645252834,  1.509069366 },
            {    0.001358e-6,    11513.883316794,  0.495572260 },
            {    0.001628e-6,     7632.943259650,  4.968445721 },
            {    0.001169e-6,      103.092774219,  2.838496795 },
            /* 551, 560 */
            {    0.001162e-6,     4164.311989613,  3.408387778 },
            {    0.001092e-6,     6069.776754553,  3.617942651 },
            {    0.001008e-6,    17789.845619785,  0.286350174 },
            {    0.001008e-6,      639.897286314,  1.610762073 },
            {    0.000918e-6,    10213.285546211,  5.532798067 },
            {    0.001011e-6,    -6256.777530192,  0.661826484 },
            {    0.000753e-6,    16730.463689596,  3.905030235 },
            {    0.000737e-6,    11926.254413669,  4.641956361 },
            {    0.000694e-6,     3340.612426700,  2.111120332 },
            {    0.000701e-6,     3894.181829542,  2.760823491 },
            /* 561, 570 */
            {    0.000689e-6,     -135.065080035,  4.768800780 },
            {    0.000700e-6,    13367.972631107,  5.760439898 },
            {    0.000664e-6,     6040.347246017,  1.051215840 },
            {    0.000654e-6,     5650.292110678,  4.911332503 },
            {    0.000788e-6,     6681.224853400,  4.699648011 },
            {    0.000628e-6,     5333.900241022,  5.024608847 },
            {    0.000755e-6,     -110.206321219,  4.370971253 },
            {    0.000628e-6,     6290.189396992,  3.660478857 },
            {    0.000635e-6,    25132.303399966,  4.121051532 },
            {    0.000534e-6,     5966.683980335,  1.173284524 },
            /* 571, 580 */
            {    0.000543e-6,     -433.711737877,  0.345585464 },
            {    0.000517e-6,    -1990.745017041,  5.414571768 },
            {    0.000504e-6,     5767.611978898,  2.328281115 },
            {    0.000485e-6,     5753.384884897,  1.685874771 },
            {    0.000463e-6,     7860.419392439,  5.297703006 },
            {    0.000604e-6,      515.463871093,  0.591998446 },
            {    0.000443e-6,    12168.002696575,  4.830881244 },
            {    0.000570e-6,      199.072001436,  3.899190272 },
            {    0.000465e-6,    10969.965257698,  0.476681802 },
            {    0.000424e-6,    -7079.373856808,  1.112242763 },
            /* 581, 590 */
            {    0.000427e-6,      735.876513532,  1.994214480 },
            {    0.000478e-6,    -6127.655450557,  3.778025483 },
            {    0.000414e-6,    10973.555686350,  5.441088327 },
            {    0.000512e-6,     1589.072895284,  0.107123853 },
            {    0.000378e-6,    10984.192351700,  0.915087231 },
            {    0.000402e-6,    11371.704689758,  4.107281715 },
            {    0.000453e-6,     9917.696874510,  1.917490952 },
            {    0.000395e-6,      149.563197135,  2.763124165 },
            {    0.000371e-6,     5739.157790895,  3.112111866 },
            {    0.000350e-6,    11790.629088659,  0.440639857 },
            /* 591, 600 */
            {    0.000356e-6,     6133.512652857,  5.444568842 },
            {    0.000344e-6,      412.371096874,  5.676832684 },
            {    0.000383e-6,      955.599741609,  5.559734846 },
            {    0.000333e-6,     6496.374945429,  0.261537984 },
            {    0.000340e-6,     6055.549660552,  5.975534987 },
            {    0.000334e-6,     1066.495477190,  2.335063907 },
            {    0.000399e-6,    11506.769769794,  5.321230910 },
            {    0.000314e-6,    18319.536584880,  2.313312404 },
            {    0.000424e-6,     1052.268383188,  1.211961766 },
            {    0.000307e-6,       63.735898303,  3.169551388 },
            /* 601, 610 */
            {    0.000329e-6,       29.821438149,  6.106912080 },
            {    0.000357e-6,     6309.374169791,  4.223760346 },
            {    0.000312e-6,    -3738.761430108,  2.180556645 },
            {    0.000301e-6,      309.278322656,  1.499984572 },
            {    0.000268e-6,    12043.574281889,  2.447520648 },
            {    0.000257e-6,    12491.370101415,  3.662331761 },
            {    0.000290e-6,      625.670192312,  1.272834584 },
            {    0.000256e-6,     5429.879468239,  1.913426912 },
            {    0.000339e-6,     3496.032826134,  4.165930011 },
            {    0.000283e-6,     3930.209696220,  4.325565754 },
            /* 611, 620 */
            {    0.000241e-6,    12528.018664345,  3.832324536 },
            {    0.000304e-6,     4686.889407707,  1.612348468 },
            {    0.000259e-6,    16200.772724501,  3.470173146 },
            {    0.000238e-6,    12139.553509107,  1.147977842 },
            {    0.000236e-6,     6172.869528772,  3.776271728 },
            {    0.000296e-6,    -7058.598461315,  0.460368852 },
            {    0.000306e-6,    10575.406682942,  0.554749016 },
            {    0.000251e-6,    17298.182327326,  0.834332510 },
            {    0.000290e-6,     4732.030627343,  4.759564091 },
            {    0.000261e-6,     5884.926846583,  0.298259862 },
            /* 621, 630 */
            {    0.000249e-6,     5547.199336460,  3.749366406 },
            {    0.000213e-6,    11712.955318231,  5.415666119 },
            {    0.000223e-6,     4701.116501708,  2.703203558 },
            {    0.000268e-6,     -640.877607382,  0.283670793 },
            {    0.000209e-6,     5636.065016677,  1.238477199 },
            {    0.000193e-6,    10177.257679534,  1.943251340 },
            {    0.000182e-6,     6283.143160294,  2.456157599 },
            {    0.000184e-6,     -227.526189440,  5.888038582 },
            {    0.000182e-6,    -6283.008539689,  0.241332086 },
            {    0.000228e-6,    -6284.056171060,  2.657323816 },
            /* 631, 640 */
            {    0.000166e-6,     7238.675591600,  5.930629110 },
            {    0.000167e-6,     3097.883822726,  5.570955333 },
            {    0.000159e-6,     -323.505416657,  5.786670700 },
            {    0.000154e-6,    -4136.910433516,  1.517805532 },
            {    0.000176e-6,    12029.347187887,  3.139266834 },
            {    0.000167e-6,    12132.439962106,  3.556352289 },
            {    0.000153e-6,      202.253395174,  1.463313961 },
            {    0.000157e-6,    17267.268201691,  1.586837396 },
            {    0.000142e-6,    83996.847317911,  0.022670115 },
            {    0.000152e-6,    17260.154654690,  0.708528947 },
            /* 641, 650 */
            {    0.000144e-6,     6084.003848555,  5.187075177 },
            {    0.000135e-6,     5756.566278634,  1.993229262 },
            {    0.000134e-6,     5750.203491159,  3.457197134 },
            {    0.000144e-6,     5326.786694021,  6.066193291 },
            {    0.000160e-6,    11015.106477335,  1.710431974 },
            {    0.000133e-6,     3634.621024518,  2.836451652 },
            {    0.000134e-6,    18073.704938650,  5.453106665 },
            {    0.000134e-6,     1162.474704408,  5.326898811 },
            {    0.000128e-6,     5642.198242609,  2.511652591 },
            {    0.000160e-6,      632.783739313,  5.628785365 },
            /* 651, 660 */
            {    0.000132e-6,    13916.019109642,  0.819294053 },
            {    0.000122e-6,    14314.168113050,  5.677408071 },
            {    0.000125e-6,    12359.966151546,  5.251984735 },
            {    0.000121e-6,     5749.452731634,  2.210924603 },
            {    0.000136e-6,     -245.831646229,  1.646502367 },
            {    0.000120e-6,     5757.317038160,  3.240883049 },
            {    0.000134e-6,    12146.667056108,  3.059480037 },
            {    0.000137e-6,     6206.809778716,  1.867105418 },
            {    0.000141e-6,    17253.041107690,  2.069217456 },
            {    0.000129e-6,    -7477.522860216,  2.781469314 },
            /* 661, 670 */
            {    0.000116e-6,     5540.085789459,  4.281176991 },
            {    0.000116e-6,     9779.108676125,  3.320925381 },
            {    0.000129e-6,     5237.921013804,  3.497704076 },
            {    0.000113e-6,     5959.570433334,  0.983210840 },
            {    0.000122e-6,     6282.095528923,  2.674938860 },
            {    0.000140e-6,      -11.045700264,  4.957936982 },
            {    0.000108e-6,    23543.230504682,  1.390113589 },
            {    0.000106e-6,   -12569.674818332,  0.429631317 },
            {    0.000110e-6,     -266.607041722,  5.501340197 },
            {    0.000115e-6,    12559.038152982,  4.691456618 },
            /* 671, 680 */
            {    0.000134e-6,    -2388.894020449,  0.577313584 },
            {    0.000109e-6,    10440.274292604,  6.218148717 },
            {    0.000102e-6,     -543.918059096,  1.477842615 },
            {    0.000108e-6,    21228.392023546,  2.237753948 },
            {    0.000101e-6,    -4535.059436924,  3.100492232 },
            {    0.000103e-6,       76.266071276,  5.594294322 },
            {    0.000104e-6,      949.175608970,  5.674287810 },
            {    0.000101e-6,    13517.870106233,  2.196632348 },
            {    0.000100e-6,    11933.367960670,  4.056084160 },
            
            /* T^2 */
            {    4.322990e-6,     6283.075849991,  2.642893748 },
            /* 681, 690 */
            {    0.406495e-6,        0.000000000,  4.712388980 },
            {    0.122605e-6,    12566.151699983,  2.438140634 },
            {    0.019476e-6,      213.299095438,  1.642186981 },
            {    0.016916e-6,      529.690965095,  4.510959344 },
            {    0.013374e-6,       -3.523118349,  1.502210314 },
            {    0.008042e-6,       26.298319800,  0.478549024 },
            {    0.007824e-6,      155.420399434,  5.254710405 },
            {    0.004894e-6,     5746.271337896,  4.683210850 },
            {    0.004875e-6,     5760.498431898,  0.759507698 },
            {    0.004416e-6,     5223.693919802,  6.028853166 },
            /* 691, 700 */
            {    0.004088e-6,       -7.113547001,  0.060926389 },
            {    0.004433e-6,    77713.771467920,  3.627734103 },
            {    0.003277e-6,    18849.227549974,  2.327912542 },
            {    0.002703e-6,     6062.663207553,  1.271941729 },
            {    0.003435e-6,     -775.522611324,  0.747446224 },
            {    0.002618e-6,     6076.890301554,  3.633715689 },
            {    0.003146e-6,      206.185548437,  5.647874613 },
            {    0.002544e-6,     1577.343542448,  6.232904270 },
            {    0.002218e-6,     -220.412642439,  1.309509946 },
            {    0.002197e-6,     5856.477659115,  2.407212349 },
            /* 701, 710 */
            {    0.002897e-6,     5753.384884897,  5.863842246 },
            {    0.001766e-6,      426.598190876,  0.754113147 },
            {    0.001738e-6,     -796.298006816,  2.714942671 },
            {    0.001695e-6,      522.577418094,  2.629369842 },
            {    0.001584e-6,     5507.553238667,  1.341138229 },
            {    0.001503e-6,     -242.728603974,  0.377699736 },
            {    0.001552e-6,     -536.804512095,  2.904684667 },
            {    0.001370e-6,     -398.149003408,  1.265599125 },
            {    0.001889e-6,    -5573.142801634,  4.413514859 },
            {    0.001722e-6,     6069.776754553,  2.445966339 },
            /* 711, 720 */
            {    0.001124e-6,     1059.381930189,  5.041799657 },
            {    0.001258e-6,      553.569402842,  3.849557278 },
            {    0.000831e-6,      951.718406251,  2.471094709 },
            {    0.000767e-6,     4694.002954708,  5.363125422 },
            {    0.000756e-6,     1349.867409659,  1.046195744 },
            {    0.000775e-6,      -11.045700264,  0.245548001 },
            {    0.000597e-6,     2146.165416475,  4.543268798 },
            {    0.000568e-6,     5216.580372801,  4.178853144 },
            {    0.000711e-6,     1748.016413067,  5.934271972 },
            {    0.000499e-6,    12036.460734888,  0.624434410 },
            /* 721, 730 */
            {    0.000671e-6,    -1194.447010225,  4.136047594 },
            {    0.000488e-6,     5849.364112115,  2.209679987 },
            {    0.000621e-6,     6438.496249426,  4.518860804 },
            {    0.000495e-6,    -6286.598968340,  1.868201275 },
            {    0.000456e-6,     5230.807466803,  1.271231591 },
            {    0.000451e-6,     5088.628839767,  0.084060889 },
            {    0.000435e-6,     5643.178563677,  3.324456609 },
            {    0.000387e-6,    10977.078804699,  4.052488477 },
            {    0.000547e-6,   161000.685737473,  2.841633844 },
            {    0.000522e-6,     3154.687084896,  2.171979966 },
            /* 731, 740 */
            {    0.000375e-6,     5486.777843175,  4.983027306 },
            {    0.000421e-6,     5863.591206116,  4.546432249 },
            {    0.000439e-6,     7084.896781115,  0.522967921 },
            {    0.000309e-6,     2544.314419883,  3.172606705 },
            {    0.000347e-6,     4690.479836359,  1.479586566 },
            {    0.000317e-6,      801.820931124,  3.553088096 },
            {    0.000262e-6,      419.484643875,  0.606635550 },
            {    0.000248e-6,     6836.645252834,  3.014082064 },
            {    0.000245e-6,    -1592.596013633,  5.519526220 },
            {    0.000225e-6,     4292.330832950,  2.877956536 },
            /* 741, 750 */
            {    0.000214e-6,     7234.794256242,  1.605227587 },
            {    0.000205e-6,     5767.611978898,  0.625804796 },
            {    0.000180e-6,    10447.387839604,  3.499954526 },
            {    0.000229e-6,      199.072001436,  5.632304604 },
            {    0.000214e-6,      639.897286314,  5.960227667 },
            {    0.000175e-6,     -433.711737877,  2.162417992 },
            {    0.000209e-6,      515.463871093,  2.322150893 },
            {    0.000173e-6,     6040.347246017,  2.556183691 },
            {    0.000184e-6,     6309.374169791,  4.732296790 },
            {    0.000227e-6,   149854.400134205,  5.385812217 },
            /* 751, 760 */
            {    0.000154e-6,     8031.092263058,  5.120720920 },
            {    0.000151e-6,     5739.157790895,  4.815000443 },
            {    0.000197e-6,     7632.943259650,  0.222827271 },
            {    0.000197e-6,       74.781598567,  3.910456770 },
            {    0.000138e-6,     6055.549660552,  1.397484253 },
            {    0.000149e-6,    -6127.655450557,  5.333727496 },
            {    0.000137e-6,     3894.181829542,  4.281749907 },
            {    0.000135e-6,     9437.762934887,  5.979971885 },
            {    0.000139e-6,    -2352.866153772,  4.715630782 },
            {    0.000142e-6,     6812.766815086,  0.513330157 },
            /* 761, 770 */
            {    0.000120e-6,    -4705.732307544,  0.194160689 },
            {    0.000131e-6,   -71430.695617928,  0.000379226 },
            {    0.000124e-6,     6279.552731642,  2.122264908 },
            {    0.000108e-6,    -6256.777530192,  0.883445696 },
            
            /* T^3 */
            {    0.143388e-6,     6283.075849991,  1.131453581 },
            {    0.006671e-6,    12566.151699983,  0.775148887 },
            {    0.001480e-6,      155.420399434,  0.480016880 },
            {    0.000934e-6,      213.299095438,  6.144453084 },
            {    0.000795e-6,      529.690965095,  2.941595619 },
            {    0.000673e-6,     5746.271337896,  0.120415406 },
            /* 771, 780 */
            {    0.000672e-6,     5760.498431898,  5.317009738 },
            {    0.000389e-6,     -220.412642439,  3.090323467 },
            {    0.000373e-6,     6062.663207553,  3.003551964 },
            {    0.000360e-6,     6076.890301554,  1.918913041 },
            {    0.000316e-6,      -21.340641002,  5.545798121 },
            {    0.000315e-6,     -242.728603974,  1.884932563 },
            {    0.000278e-6,      206.185548437,  1.266254859 },
            {    0.000238e-6,     -536.804512095,  4.532664830 },
            {    0.000185e-6,      522.577418094,  4.578313856 },
            {    0.000245e-6,    18849.227549974,  0.587467082 },
            /* 781, 787 */
            {    0.000180e-6,      426.598190876,  5.151178553 },
            {    0.000200e-6,      553.569402842,  5.355983739 },
            {    0.000141e-6,     5223.693919802,  1.336556009 },
            {    0.000104e-6,     5856.477659115,  4.239842759 },
            
            /* T^4 */
            {    0.003826e-6,     6283.075849991,  5.705257275 },
            {    0.000303e-6,    12566.151699983,  5.407132842 },
            {    0.000209e-6,      155.420399434,  1.989815753 }
        };
        
        /* Time since J2000.0 in Julian millennia. */
        t = ((date1 - DJ00) + date2) / DJM;
        
        /* ================= */
        /* Topocentric terms */
        /* ================= */
        
        /* Convert UT to local solar time in radians. */
        tsol = fmod(ut, 1.0) * D2PI + elong;
        
        /* FUNDAMENTAL ARGUMENTS:  Simon et al. 1994. */
        
        /* Combine time argument (millennia) with deg/arcsec factor. */
        w = t / 3600.0;
        
        /* Sun Mean Longitude. */
        elsun = fmod(280.46645683 + 1296027711.03429 * w, 360.0) * DD2R;
        
        /* Sun Mean Anomaly. */
        emsun = fmod(357.52910918 + 1295965810.481 * w, 360.0) * DD2R;
        
        /* Mean Elongation of Moon from Sun. */
        d = fmod(297.85019547 + 16029616012.090 * w, 360.0) * DD2R;
        
        /* Mean Longitude of Jupiter. */
        elj = fmod(34.35151874 + 109306899.89453 * w, 360.0) * DD2R;
        
        /* Mean Longitude of Saturn. */
        els = fmod(50.07744430 + 44046398.47038 * w, 360.0) * DD2R;
        
        /* TOPOCENTRIC TERMS:  Moyer 1981 and Murray 1983. */
        wt =   +  0.00029e-10 * u * sin(tsol + elsun - els)
        +  0.00100e-10 * u * sin(tsol - 2.0 * emsun)
        +  0.00133e-10 * u * sin(tsol - d)
        +  0.00133e-10 * u * sin(tsol + elsun - elj)
        -  0.00229e-10 * u * sin(tsol + 2.0 * elsun + emsun)
        -  0.02200e-10 * v * cos(elsun + emsun)
        +  0.05312e-10 * u * sin(tsol - emsun)
        -  0.13677e-10 * u * sin(tsol + 2.0 * elsun)
        -  1.31840e-10 * v * cos(elsun)
        +  3.17679e-10 * u * sin(tsol);
        
        /* ===================== */
        /* Fairhead et al. model */
        /* ===================== */
        
        /* T**0 */
        w0 = 0;
        for (j = 473; j >= 0; j--) {
            w0 += fairhd[j][0] * sin(fairhd[j][1] * t + fairhd[j][2]);
        }
        
        /* T**1 */
        w1 = 0;
        for (j = 678; j >= 474; j--) {
            w1 += fairhd[j][0] * sin(fairhd[j][1] * t + fairhd[j][2]);
        }
        
        /* T**2 */
        w2 = 0;
        for (j = 763; j >= 679; j--) {
            w2 += fairhd[j][0] * sin(fairhd[j][1] * t + fairhd[j][2]);
        }
        
        /* T**3 */
        w3 = 0;
        for (j = 783; j >= 764; j--) {
            w3 += fairhd[j][0] * sin(fairhd[j][1] * t + fairhd[j][2]);
        }
        
        /* T**4 */
        w4 = 0;
        for (j = 786; j >= 784; j--) {
            w4 += fairhd[j][0] * sin(fairhd[j][1] * t + fairhd[j][2]);
        }
        
        /* Multiply by powers of T and combine. */
        wf = t * (t * (t * (t * w4 + w3) + w2) + w1) + w0;
        
        /* Adjustments to use JPL planetary masses instead of IAU. */
        wj =   0.00065e-6 * sin(6069.776754 * t + 4.021194) +
        0.00033e-6 * sin( 213.299095 * t + 5.543132) +
        (-0.00196e-6 * sin(6208.294251 * t + 5.696701)) +
        (-0.00173e-6 * sin(  74.781599 * t + 2.435900)) +
        0.03638e-6 * t * t;
        
        /* ============ */
        /* Final result */
        /* ============ */
        
        /* TDB-TT in seconds. */
        w = wt + wf + wj;
        
        return w;
        
    }
    
    
    
    /* Function to convert from UTC to sidereal time
     * @param t         Epoch
     *
     * @return sidereal time in hours.
     */
    double GTime::UTC2SID(GTime UTC)
    {
        double sid = 0.0;
        
        
        JDTime jdt = GTime::GTime2JDTime(UTC);
        // Compute Julian Day, including decimals
        double jd = jdt.jdt();
        
        CivilTime cvt = GTime::JDTime2CivilTime(jdt);
        DOYTime doyt = GTime::CivilTime2DOYTime(cvt);
        
        double h = doyt.m_sod/GCONST("SECPHOR");
        
        //const YDSTime yds(t);
        // Hours of day (decimal)
        //double h(yds.sod/3600.0);
        
        // Compute Julian Day, including decimals
        //double jd(JulianDate(t).jd);
        
        // Temporal value, in centuries
        double tt( (jd - 2451545.0)/36525.0 );
        
        sid = ( 24110.54841 + tt*( (8640184.812866) +
                                  tt*( (0.093104) - (6.2e-6*tt)) ) );
        
        sid = sid/3600.0 + h;
        sid = ::fmod(sid,24.0);
        
        if (sid < 0.0)
        {
            sid+=24.0;
        }
        
        return sid ;
    }
    
    /*算法参考： GPS Theory and Practice , B.Hofmann-Wellenhof,H.Lichtenegger, J.Collins
     fifth, revised edition
     */
    JDTime GTime::CivilTime2JDTime(CivilTime ct)
    {
        long_t MINPHOR = GCONST("MINPHOR");
        long_t SECPHOR = GCONST("SECPHOR");
        long_t HORPDAY = GCONST("HORPDAY");
        long_t SECPMIN = GCONST("SECPMIN");
        
        JDTime jdt;
        jdt.m_ts = ct.m_ts;
        
        int y = 0 , m = 0;
        y = ct.m_year;m = ct.m_month;
        if( ct.m_month <= 2 )
        {
            y = y-1; m = m+12;
        }
        
        double	ut = ct.m_hour + ct.m_minute/MINPHOR+ ct.m_second/SECPHOR;
        long double tmpjd = static_cast<long>(365.25*y) + static_cast<long>(30.6001*(m+1))+ ct.m_day + ut/HORPDAY + 1720981.5;
        
        jdt.m_jd = static_cast<long>(tmpjd);  //这里直接取整，错误！！！！
        
        jdt.m_sod = ( (ct.m_hour + 12)%HORPDAY )*SECPHOR + ct.m_minute*SECPMIN	+ static_cast<long>(ct.m_second);
        
        jdt.m_fsod = ct.m_second - static_cast<long>(ct.m_second) ;
        
        return jdt;
    }
    
    /*fsodp should be in seconds*/
    void GTime::SetData(TimeSystem tsp, long mjdp,long sodp, double fsodp)
    {
        double NANO = GCONST("NANO");
        //为了保证数据的正确性，必须保证sodp<86400;且fsodp < 1.0E9
        if( sodp >= GCONST("SECPDAY") && fsodp > 1.0 )
        {
            //非法请求
            InvalidRequest ir("Time Parameters out of range!");
            GFC_THROW( ir );
        }
        
        m_ts  = tsp;
        m_mjd = mjdp;
        m_sod = sodp;
        m_fsod = fsodp*NANO;
    }
    
    /*fsod should be in seconds*/
    void GTime::GetData( TimeSystem& ts, long& mjdp,long& sodp, double& fsodp)
    {
        double NANO = GCONST("NANO");
        
        ts = m_ts;
        mjdp = m_mjd;
        sodp = m_sod;
        fsodp = m_fsod/NANO;
        
    }
    
    
    /*年月日 转换为  GTime*/
    GTime GTime::CivilTime2GTime( CivilTime ct )
    {
        JDTime jd = CivilTime2JDTime(ct);
        GTime  gt = JDTime2GTime(jd);
        return gt;
    }
    
    /*年月日 转换为 DOYT */
    DOYTime GTime::CivilTime2DOYTime( CivilTime ct )
    {
        DOYTime dt;
        dt.m_ts = ct.m_ts;
        CivilTime ct1; ct1.m_ts = ct.m_ts; ct1.m_year = ct.m_year ; ct1.m_month = 1; ct1.m_day = 1;
        GTime gt1 = CivilTime2GTime(ct);
        GTime gt2 = CivilTime2GTime(ct1);  //当前年的第一天
        GTime gt = gt1 - gt2 ;             //需要重载减法运算
        //需要将GTime gt转换为DOYTime
        dt.m_year = ct.m_year;
        
        dt.m_doy = (int)gt.toDays();
        dt.m_sod = gt.toSeconds() - dt.m_doy*86400.0;
        
        return dt;
    }
    
    
    /*将儒略日转换为简化儒略日*/
    GTime GTime::JDTime2GTime( JDTime jdt )
    {
        GTime  gt;
        
        double mjd0 = GCONST("MJD0");
        long_t SECPDAY = GCONST("SECPDAY");
        long_t NANO = GCONST("NANO");
        double tmpjd = jdt.m_jd;
        double tmpsod = jdt.m_sod;
        if( jdt.m_sod+jdt.m_fsod - 0.5*SECPDAY < 0   )
        {
            tmpsod  =  jdt.m_sod + SECPDAY ; //整数秒的变化
            tmpjd   =  jdt.m_jd - 1 ;  //这时 jd变成了小数
        }
        //
        tmpsod = tmpsod - 0.5*SECPDAY;
        tmpjd =  tmpjd - mjd0+0.5;
        
        double tmpfsod = jdt.m_fsod;
        
        gt.SetData(jdt.m_ts,tmpjd,tmpsod,tmpfsod);
        
        return gt;
        
    }
    
    
    // convert from GTime to JDCentury
    // here gt should be in TT timesystem
    // //compute number of Julian centuries since J2000
    double GTime::JDCenturySince2000( GTime gt )
    {
        GTime j2000 = GTime::J2000();
        GTime t = gt-j2000;
        long mjd =0, sod=0;
        double fsod =0.0;
        TimeSystem ts;
        t.GetData(ts, mjd, sod, fsod);
        long secpday = GCONST("SECPDAY");
        return (mjd+ (sod + fsod)/secpday)/36525.0;
        
        //JDTime jdt = gt.GTime2JDTime(gt);
        //long  secpday = GCONST("SECPDAY");
        //long double tmp = jdt.m_jd + (jdt.m_sod + jdt.m_fsod)/secpday;
        //tmp = tmp - 2451545.0; // here 2451545.0 is the Julian Day of J2000 ???
        //return (tmp/36525.0);
    }
    
    CivilTime GTime::GTime2CivilTime(GTime gt)
    {
        JDTime jdt = GTime::GTime2JDTime(gt);
        CivilTime ct = GTime::JDTime2CivilTime(jdt);
        
        return ct;
    }
    
    
    
    NavTime GTime::GTime2NavTime(GTime gt)
    {
        NavTime nt;
        
        GTime T0;
        if( gt.getTimeSystem().getTimeSystemName() == "tsGPS" )
        {
            T0 = GPST0();
        }
        else if(gt.getTimeSystem().getTimeSystemName() == "tsBDS")
        {
            T0 = BDT0();
        }
        
        int RemainDay = 0;
        
        double d1 = gt.getMJD() + (gt.getSOD() + gt.getFSOD())/86400.0;
        double d0 = T0.getMJD() + (T0.getSOD() + T0.getFSOD())/86400.0;
        
        nt.m_week = (int)(   ( d1 - d0 ) / 7 );
        
        RemainDay = (int)(d1 - d0 - nt.m_week * 7  );
        
        //NAVT.SecOfWeek = ( RemainDay + (MJDT.Second + MJDT.FracSec)/SEC_DAY ) * SEC_DAY;
        
        nt.m_sow = ( RemainDay*86400.0 + gt.getSOD() + gt.getFSOD()) ;
        
        nt.m_ts = T0.getTimeSystem();
        
        return nt;
    }
    
    
    //MJD2JD
    //MJD = JD - 2400000.5
    // JD = MJD + 2400000.5
    JDTime GTime::GTime2JDTime( GTime gt )
    {
        long_t SECPDAY = GCONST("SECPDAY");
        
        JDTime  jdt;
        long mjdp = 0, sodp = 0;
        double fsodp = 0.0, mjd0 = 0.0;
        mjd0 = GCONST("MJD0");
        gt.GetData(jdt.m_ts,mjdp,sodp,fsodp);
        double nano = GCONST("NANO");
        double tmpsec = sodp + fsodp;  //以秒为单位的天内秒
        if( tmpsec >= 0.5*SECPDAY )  //秒大于半天
        {
            mjdp = (long)(mjdp + mjd0 + 0.5);  //变为整数
            sodp = sodp - 0.5*SECPDAY;
        }
        else
        {
            mjdp = (long)(mjdp + mjd0-0.5);
            sodp = sodp + 0.5*SECPDAY;
        }
        
        jdt.m_jd = mjdp;
        jdt.m_sod = sodp;
        jdt.m_fsod = fsodp;
        
        return jdt;
    }
    
    /*
     静态函数 JDT2CivilTime
     算法参考： GPS Theory and Practice , B.Hofmann-Wellenhof,H.Lichtenegger, J.Collins
     fifth, revised edition
     */
    CivilTime GTime::JDTime2CivilTime( JDTime jdt )
    {
        
        long_t  SECPDAY = GCONST("SECPDAY");
        long_t  SECPHOR = GCONST("SECPHOR");
        long_t  SECPMIN = GCONST("SECPMIN");
        
        CivilTime ct;
        ct.m_ts = jdt.m_ts;  //时间系统赋值
        int iday= 0, imonth =0, iyear = 0;
        
        //将jdt加上0.5天
        if( jdt.m_sod >= static_cast<long>(0.5*SECPDAY) )
        {
            jdt.m_jd = jdt.m_jd  + 1;
            jdt.m_sod = jdt.m_sod - 0.5*SECPDAY;
        }
        else
        {
            jdt.m_sod = jdt.m_sod + static_cast<long>(0.5*SECPDAY);
        }
        
        long L, M, N, P, Q;
        if( jdt.m_jd > 2299160 )    // after Oct 4, 1582
        {
            L = jdt.m_jd + 68569;
            M = (4 * L) / 146097;
            L = L - ((146097 * M + 3) / 4);
            N = (4000 * (L + 1)) / 1461001;
            L = L - ((1461 * N) / 4) + 31;
            P = (80 * L) / 2447;
            iday = int(L - (2447 * P) / 80);
            L = P / 11;
            imonth = int(P + 2 - 12 * L);
            iyear =  int(100 * (M - 49) + N + L);
        }
        else
        {
            P = jdt.m_jd + 1402;
            Q = (P - 1) / 1461;
            L = P - 1461 * Q;
            M = (L - 1) / 365 - L / 1461;
            N = L - 365 * M + 30;
            P = (80 * N) / 2447;
            iday = int(N - (2447 * P) / 80);
            N = P / 11;
            imonth = int(P + 2 - 12 * N);
            iyear = int(4 * Q + M + N - 4716);
            if( iyear <= 0 )
            {
                --iyear;
            }
        }
        // catch century/non-400 non-leap years
        if(iyear > 1599 &&
           !(iyear % 100) &&
           (iyear % 400) &&
           imonth == 2 &&
           iday == 29)
        {
            imonth = 3;
            iday = 1;
        }
        
        ct.m_year = iyear;
        ct.m_month = imonth;
        ct.m_day = iday;
        
//        
//        
//        long a = 0, b = 0 , c = 0 , d = 0 , e = 0;
//        
//        double fracJd = jdt.m_jd  + (jdt.m_sod + jdt.m_fsod)/SECPDAY;
//        
//        a = static_cast<long>( fracJd + 0.5 );
//        b = a+ 1537;
//        c = static_cast<long>( ( b - 122.1 )/365.25 );
//        d = static_cast<long>( 365.25 * c );
//        e = static_cast<long>( ( b-d )/30.6001 );
//        
//        
//        ct.m_month = static_cast<int_t>(e -1 -12 * static_cast<long>( e/14 )) ;
//        ct.m_year  = static_cast<int_t>(c - 4715 - static_cast<long>( ( 7 + ct.m_month )/10 ));
//        ct.m_day  = static_cast<int_t>( b - d -static_cast<long>( 30.6001 * e ));
//        
        
        ct.m_hour  =  static_cast<int_t>( (jdt.m_sod + jdt.m_fsod)/SECPHOR );
        
        ct.m_minute = static_cast<int_t>( ( jdt.m_sod + jdt.m_fsod - ct.m_hour*SECPHOR )/SECPMIN );
        
        ct.m_second = jdt.m_sod - ct.m_hour*SECPHOR - ct.m_minute*SECPMIN + jdt.m_fsod;
        
        return ct;
        
    }
    
    /*赋值重载函数*/
    GTime& GTime::operator =( const GTime& right )
    {
        this->m_ts = right.m_ts;
        this->m_mjd = right.m_mjd;
        this->m_sod = right.m_sod;
        this->m_fsod = right.m_fsod;
        return *this;
    }
    
    /*加法重载*/
    GTime GTime::operator+ ( const GTime& right ) const
    {
        long_t NANO = GCONST("NANO");
        long_t SECPDAY = GCONST("SECPDAY");
        GTime  gt;
        if( ( m_ts != GTimeSystem("tsUKN") &&
             right.m_ts != GTimeSystem("tsUKN")) &&
           m_ts != right.m_ts )
        {
            //非法请求
            InvalidRequest ir("GTime objects not in same time system, cannot be differenced");
            GFC_THROW( ir );
        }
        
        gt.m_fsod = this->m_fsod + right.m_fsod;
        if( gt.m_fsod >= NANO )
        {
            gt.m_fsod = gt.m_fsod - NANO;
            gt.m_sod = gt.m_sod + 1;
        }
        
        gt.m_sod = gt.m_sod + ( this->m_sod + right.m_sod );
        if(gt.m_sod >= SECPDAY )
        {
            gt.m_sod = gt.m_sod - SECPDAY;
            gt.m_mjd = gt.m_mjd + 1;
        }
        
        gt.m_mjd = gt.m_mjd + ( this->m_mjd + right.m_mjd );
        
        gt.m_ts = m_ts;
        return gt;
    }
    
    GTime GTime::operator+ ( const double& second ) const
    {
        long NANO = GCONST("NANO");
        long SECPDAY = GCONST("SECPDAY");
        GTime gt;
        
        if(second >=0.0)
        {
            long tmjd = m_mjd;
            long tsod = m_sod;
            double tfsod = m_fsod; //unit: nanosecond
            
            long isec = static_cast<long>(second);
            long double nanosecond = (second-isec)*NANO;
            
            tsod +=  isec;
            double tmpfsod = tfsod + nanosecond;
            long test1 = static_cast<long> (tmpfsod) / static_cast<long>(NANO);
            tsod  = tsod + test1;
            
            tfsod  = tmpfsod - test1*NANO;
            
            long temp1 = static_cast<long>(tsod) / static_cast<long>(SECPDAY); // 第2次进位 , uint: day
            tmjd = tmjd + temp1;
            tsod = (tsod - temp1*SECPDAY);
             gt.SetData(m_ts,tmjd,tsod,tfsod/NANO);
            
        }
        else if(second <=0.0)
        {
            double  mysecond = -second;
            gt = *this - mysecond;
        }
        
        return gt;
    }
    
    /*减法重载*/
    GTime GTime::operator -( const GTime& right ) const
    {
        long_t NANO = GCONST("NANO");
        long_t SECPDAY = GCONST("SECPDAY");
        GTime ct = *this;
        GTime  gt;
        if( ( m_ts!= GTimeSystem("tsUKN") &&
             right.m_ts != GTimeSystem("tsUKN")) &&
           m_ts != right.m_ts )
        {
            //非法请求
            InvalidRequest ir("GTime objects not in same time system, cannot be differenced");
            GFC_THROW( ir );
        }
        
        //模拟手动进位计算过程；进一位可以保证计算的正确性
        if( ct.m_fsod < right.m_fsod )
        {
            ct.m_sod = ct.m_sod -1;
            ct.m_fsod = ct.m_fsod + NANO;
        }
        
        gt.m_fsod = ct.m_fsod - right.m_fsod;
        //fsod的计算完成
        if( ct.m_sod < right.m_sod )
        {
            ct.m_mjd = ct.m_mjd - 1;
            ct.m_sod = ct.m_sod + SECPDAY;
        }
        
        gt.m_sod = ct.m_sod - right.m_sod;
        //秒的计算完成
        //开始计算天
        gt.m_mjd = ct.m_mjd - right.m_mjd ;
        gt.m_ts = m_ts;
        
        return gt;
        
    }
    
    /*减法重载*/
    GTime GTime::operator -( const double& second ) const
    {
        long_t NANO = GCONST("NANO");
        long_t SECPDAY = GCONST("SECPDAY");
        GTime ct= *this;
        GTime gt;
        
        long double nanosecond = second*NANO;
        if(nanosecond > 0.0 )
        {
            long double temp = ct.m_fsod;
            while(1)
            {
                if( temp - nanosecond >= 0.0 )
                {
                    break;
                }
                ct.m_sod = ct.m_sod - 1.0;  // one second
                temp = temp + NANO;
                if( ct.m_sod < 0.0 )
                {
                    ct.m_mjd = ct.m_mjd -1.0;  // one day
                    ct.m_sod = ct.m_sod + SECPDAY;
                }
            }
            
            //开始对数据进行整理(tmjd, tsod, tfsod)
            gt.SetData(m_ts,ct.m_mjd,ct.m_sod,(temp - nanosecond)/NANO);
        }
        else if( second <= 0.0)
        {
            double  mysecond = -second;;
            gt = *this + mysecond;
        }
        
        return gt;
    }
    
    bool GTime::operator==(const gfc::GTime &right) const
    {
        if( m_ts ==  right.m_ts
           && m_mjd  == right.m_mjd
           && m_sod == right.m_sod
           && fabs(m_fsod - right.m_fsod )<GTime::eps )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
//    /*等号重载*/
//    bool operator == ( const GTime& left, const GTime& right )
//    {
//        
//        if( left.getTimeSystem() == right.getTimeSystem()
//           && left.getMJD()  == right.getMJD()
//           && left.getSOD() == right.getSOD()
//           && fabs(left.getFSOD() - right.getFSOD())<GTime::eps )
//        {
//            return true;
//        }
//        else
//        {
//            return false;
//        }
//    }
    
    
    /*不等号重载*/
    bool GTime::operator!= ( const GTime& right ) const
    {
        return !operator==(right);
    }
    
//    /*不等号重载*/
//    bool operator!= ( const GTime& left, const GTime& right )
//    {
//        bool t = left==right;
//        return !t;
//    }
    
    
    
    /*
     大于号重载
     同一个时间系统的时间才能比较大小
     */
    bool GTime::operator> ( const GTime& right) const
    {
        if(  m_ts != GTimeSystem("tsUKN")
           &&right.m_ts != GTimeSystem("tsUKN")
           &&m_ts != right.m_ts
           )
        {
            //非法请求
            InvalidRequest ir("GTime objects not in same time system, cannot be differenced");
            GFC_THROW( ir );
        }
        
        if(   m_mjd > right.m_mjd
           ||(m_mjd== right.m_mjd && m_sod > right.m_sod )
           ||( m_mjd == right.m_mjd && m_sod == right.m_sod && m_fsod-right.m_fsod > GTime::eps )
           )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    
//    /*
//     大于号重载
//     同一个时间系统的时间才能比较大小
//     */
//    bool operator> ( const GTime& left, const GTime& right)
//    {
//        if(  left.getTimeSystem() != GTimeSystem("tsUKN")
//           &&right.getTimeSystem() != GTimeSystem("tsUKN")
//           &&left.getTimeSystem() != right.getTimeSystem()
//           )
//        {
//            //非法请求
//            InvalidRequest ir("GTime objects not in same time system, cannot be differenced");
//            GFC_THROW( ir );
//        }	
//        
//        if(   left.getMJD() > right.getMJD()
//           ||(left.getMJD()== right.getMJD() && left.getSOD() > right.getSOD() )
//           ||( left.getMJD() == right.getMJD() && left.getSOD() == right.getSOD() && left.getFSOD()-right.getFSOD() > GTime::eps )
//           )
//        {
//            return true;
//        }
//        else
//        {
//            return false;
//        }		
//    }
    
    
    bool GTime::operator< ( const GTime& right ) const
    {
        return !operator>=(right);
    }
//    
//    bool operator< (  const GTime& left, const GTime& right )
//    {
//        bool t = left>right;
//        return !t;
//    }
    
    
    bool GTime::operator<= ( const GTime& right ) const
    {
        return ( operator<(right) || operator==(right) );
    }
    
    /**/
//    bool operator<= (  const GTime& left, const GTime& right )
//    {
//        bool t = ( left<right || left==right );
//        return t;
//    }
    
    bool GTime::operator>= (const GTime& right ) const
    {
        return ( operator>(right) || operator==(right) );
    }
    
//    bool operator>= ( const GTime& left, const GTime& right )
//    {
//        bool t = ( left>right || left==right );
//        return t;
//    }
    
    /*
     从年月日转换过来
     从double[6]和TimeSystem进行时间形式转换
     参考：gpstk中函数 convertCalendarToJD
     */
    void GTime::SetFromCivilTime(  CivilTime ct )
    {
        JDTime  jdt = CivilTime2JDTime(ct);
        *this = JDTime2GTime(jdt);
    }
    
    /*从年和年积日转换过来，只进行时间形式的转换*/	
    void GTime::SetFromDoyTime( DOYTime dt )
    {
        //m_ts = dt.m_ts;
        //待完善
        CivilTime ct0;
        ct0.m_year = dt.m_year;
        ct0.m_month = 1;
        ct0.m_day =1;
        ct0.m_minute = 0;
        ct0.m_second =0;
        ct0.m_ts = dt.m_ts; //( dt.m_year,1,1,0,0,0.0);
        GTime gt = GTime::CivilTime2GTime(ct0);
        gt = gt + (dt.m_doy - 1)*86400.0 + dt.m_sod;
        
        *this = gt;
    }
    
    void GTime::SetFromNavTime(NavTime nt)
    {
        
        m_ts = nt.m_ts;   //时间系统赋值
        int secpday = GCONST("SECPDAY");
        int daypwek = GCONST("DAYPWEK");
        long_t  NANO = GCONST("NANO");
        
        int  deltad = 0;
        int  dow = 0;
        while ( nt.m_sow >= secpday )
        {
            nt.m_sow-=secpday;
            dow++;
        }
        while ( nt.m_sow <  0 )
        {
            nt.m_sow+=secpday;
            dow--;
        }
        
        deltad = nt.m_week*daypwek + dow;
        
        double T0 = 0.0;
        if( nt.m_ts == GTimeSystem("tsGPS"))
        {
            T0 = 44244.0;
        }
        else if( nt.m_ts == GTimeSystem("tsBDS"))
        {
            //待确定??
            T0 = 53736.0;
        }
        else if(nt.m_ts  == GTimeSystem("tsGAL"))
        {
            T0 = 51412.0;    /* 1999,8,22,0,0,0 */
        }
        
        
        m_mjd  = static_cast<long>( T0 + deltad );   //这里的44244是GPST0时刻所对应的MJD值，不同系统不一样
        m_sod  = static_cast<long>(nt.m_sow);
        m_fsod = static_cast<double>((nt.m_sow - m_sod)*NANO);   //秒的小数部分的单位是纳秒
        
    } // end of function SetFromNavTime(NavTime nt)
    
    
    /*
     * transform GTime to seconds
     */
    double GTime::toSeconds()
    {
        double sec = 0.0;
        sec = m_mjd*86400.0 + m_sod + m_fsod/1000000000.0;
        return sec;
    }
    
    double GTime::toDays()
    {
        double ret = m_mjd + (m_sod + m_fsod/1000000000.0)/86400.0;
        return ret;
    }
    
    
    
    
}  // end of namespace
