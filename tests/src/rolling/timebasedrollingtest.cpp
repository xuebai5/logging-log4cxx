/*
 * Copyright 1999,2005 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if 0

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/logger.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/helpers/simpledateformat.h>
#include <apr_time.h>
#include <iostream>
#include <log4cxx/helpers/stringhelper.h>
#include "../util/compare.h"


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::rolling;

/**
 * A rather exhaustive set of tests. Tests include leaving the ActiveFileName
 * argument blank, or setting it, with and without compression, and tests
 * with or without stopping/restarting the RollingFileAppender.
 * 
 * The regression tests log a few times using a RollingFileAppender. Then, 
 * they predict the names of the files which sould be generated and compare
 * them with witness files.
 * 
 * <pre>
         Compression    ActiveFileName  Stop/Restart 
 Test1      NO              BLANK          NO
 Test2      NO              BLANK          YES
 Test3      YES             BLANK          NO
 Test4      NO                SET          YES 
 Test5      NO                SET          NO
 Test6      YES               SET          NO
 * </pre>
 * @author Ceki G&uuml;lc&uuml;
 */
class TimeBasedRollingTest  : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE(TimeBasedRollingTest);
	CPPUNIT_TEST_SUITE_END();

    static LoggerPtr logger;

public:

  void setUp() {
      LoggerPtr root(Logger::getRootLogger());
    root->addAppender(
      new ConsoleAppender(new PatternLayout(
         LOG4CXX_STR("%d{ABSOLUTE} [%t] %level %c{2}#%M:%L - %m%n"))));
  }

  void tearDown() {
      LogManager::shutdown();
  }

  /**
   * Test rolling without compression, activeFileName left blank, no stop/start
   */
  void test1()  {
    PatternLayoutPtr layout(new PatternLayout(LOG4CXX_STR("%c{1} - %m%n")));
    RollingFileAppenderPtr rfa(new RollingFileAppender());
    rfa->setLayout(layout);

    LogString datePattern(LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss"));

    TimeBasedRollingPolicyPtr tbrp(new TimeBasedRollingPolicy());
    tbrp->setFileNamePattern(LOG4CXX_STR("output/test1-%d{yyyy-MM-dd_HH_mm_ss}"));
    Pool p;
    tbrp->activateOptions(p);
    rfa->setRollingPolicy(tbrp);
    rfa->activateOptions(p);
    logger->addAppender(rfa);

    SimpleDateFormat sdf(datePattern);
    LogString filenames[4];

    Pool pool;
    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 4; i++) {
      sdf.format(filenames[i], now, p);
      now += APR_USEC_PER_SEC;
    } }

    std::cout << "Waiting until next second and 100 millis.";
    delayUntilNextSecond(100);
    std::cout << "Done waiting.";

    { for (int i = 0; i < 5; i++) {
        std::string message("Hello---");
        StringHelper::toString(i, pool, message);
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(500000);
    } }

    std::ostringstream os;
    for (int i = 0; i < 4; i++) {
      os.str("witness/rolling/tbr-test1.");
      os << i;
      CPPUNIT_ASSERT(Compare::compare(filenames[i], File(os.str())));
    }
  }

#if 0

  /**
   * No compression, with stop/restart, activeFileName left blank
   */
  public void test2() throws Exception {
    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    PatternLayout layout1 = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa1 = new RollingFileAppender();
    rfa1.setLayout(layout1);

    TimeBasedRollingPolicy tbrp1 = new TimeBasedRollingPolicy();
    tbrp1.setFileNamePattern("output/test2-%d{" + datePattern + "}");
    tbrp1.activateOptions();
    rfa1.setRollingPolicy(tbrp1);
    rfa1.activateOptions();
    logger.addAppender(rfa1);

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 4; i++) {
      filenames[i] = "output/test2-" + sdf.format(cal.getTime());
      cal.add(Calendar.SECOND, 1);
    }

    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i <= 2; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    logger.removeAppender(rfa1);
    rfa1.close();

    PatternLayout layout2 = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa2 = new RollingFileAppender();
    rfa2.setLayout(layout2);

    TimeBasedRollingPolicy tbrp2 = new TimeBasedRollingPolicy();
    tbrp2.setFileNamePattern("output/test2-%d{" + datePattern + "}");
    tbrp2.activateOptions();
    rfa2.setRollingPolicy(tbrp2);
    rfa2.activateOptions();
    logger.addAppender(rfa2);

    for (int i = 3; i <= 4; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      assertTrue(Compare.compare(filenames[i], "witness/rolling/tbr-test2." + i));
    }
  }

  /**
   * With compression, activeFileName left blank, no stop/restart
   */
  public void test3() throws Exception {
    PatternLayout layout = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa = new RollingFileAppender();
    rfa.setLayout(layout);

    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    TimeBasedRollingPolicy tbrp = new TimeBasedRollingPolicy();
    tbrp.setFileNamePattern("output/test3-%d{" + datePattern + "}.gz");
    tbrp.activateOptions();
    rfa.setRollingPolicy(tbrp);
    rfa.activateOptions();
    logger.addAppender(rfa);

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 3; i++) {
      filenames[i] = "output/test3-" + sdf.format(cal.getTime()) + ".gz";
      cal.add(Calendar.SECOND, 1);
    }

    filenames[3] = "output/test3-" + sdf.format(cal.getTime());

    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i < 5; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      //System.out.println(i + " expected filename [" + filenames[i] + "].");
    }

    for (int i = 0; i < 3; i++) {
      assertTrue(Compare.gzCompare(filenames[i], "witness/rolling/tbr-test3." + i + ".gz"));
    }

    assertTrue(Compare.compare(filenames[3], "witness/rolling/tbr-test3.3"));
  }

  /**
   * Without compression, activeFileName set,  with stop/restart
   */
  public void test4() throws Exception {
    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    PatternLayout layout1 = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa1 = new RollingFileAppender();
    rfa1.setLayout(layout1);

    TimeBasedRollingPolicy tbrp1 = new TimeBasedRollingPolicy();
    tbrp1.setActiveFileName("output/test4.log");
    tbrp1.setFileNamePattern("output/test4-%d{" + datePattern + "}");
    tbrp1.activateOptions();
    rfa1.setRollingPolicy(tbrp1);
    rfa1.activateOptions();
    logger.addAppender(rfa1);

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 3; i++) {
      filenames[i] = "output/test4-" + sdf.format(cal.getTime());
      cal.add(Calendar.SECOND, 1);
    }
    filenames[3] = "output/test4.log";
    
    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i <= 2; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    logger.removeAppender(rfa1);
    rfa1.close();

    PatternLayout layout2 = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa2 = new RollingFileAppender();
    rfa2.setLayout(layout2);

    TimeBasedRollingPolicy tbrp2 = new TimeBasedRollingPolicy();
    tbrp2.setFileNamePattern("output/test4-%d{" + datePattern + "}");
    tbrp2.setActiveFileName("output/test4.log");
    tbrp2.activateOptions();
    rfa2.setRollingPolicy(tbrp2);
    rfa2.activateOptions();
    logger.addAppender(rfa2);

    for (int i = 3; i <= 4; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      assertTrue(Compare.compare(filenames[i], "witness/rolling/tbr-test4." + i));
    }
  }

  /**
   * No compression, activeFileName set,  without stop/restart
   */
  public void test5() throws Exception {
    PatternLayout layout = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa = new RollingFileAppender();
    rfa.setLayout(layout);

    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    TimeBasedRollingPolicy tbrp = new TimeBasedRollingPolicy();
    tbrp.setFileNamePattern("output/test5-%d{" + datePattern + "}");
    tbrp.setActiveFileName("output/test5.log");
    tbrp.activateOptions();
    rfa.setRollingPolicy(tbrp);
    rfa.activateOptions();
    logger.addAppender(rfa);

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 3; i++) {
      filenames[i] = "output/test5-" + sdf.format(cal.getTime());
      cal.add(Calendar.SECOND, 1);
    }

    filenames[3] = "output/test5.log";

    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i < 5; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      assertTrue(Compare.compare(filenames[i], "witness/rolling/tbr-test5." + i));
    }
  }

  /**
   * With compression, activeFileName set, no stop/restart,
   */
  public void test6() throws Exception {
    PatternLayout layout = new PatternLayout("%c{1} - %m%n");
    RollingFileAppender rfa = new RollingFileAppender();
    rfa.setLayout(layout);

    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    TimeBasedRollingPolicy tbrp = new TimeBasedRollingPolicy();
    tbrp.setFileNamePattern("output/test6-%d{" + datePattern + "}.gz");
    tbrp.setActiveFileName("output/test6.log");
    tbrp.activateOptions();
    rfa.setRollingPolicy(tbrp);
    rfa.activateOptions();
    logger.addAppender(rfa);

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 3; i++) {
      filenames[i] = "output/test6-" + sdf.format(cal.getTime()) + ".gz";
      cal.add(Calendar.SECOND, 1);
    }

    filenames[3] = "output/test6.log";

    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i < 5; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      //System.out.println(i + " expected filename [" + filenames[i] + "].");
    }

    for (int i = 0; i < 3; i++) {
      assertTrue(Compare.gzCompare(filenames[i], "witness/rolling/tbr-test6." + i + ".gz"));
    }

    assertTrue(Compare.compare(filenames[3], "witness/rolling/tbr-test6.3"));
  }

  public void testWithJoran1() throws Exception {
    JoranConfigurator jc = new JoranConfigurator();
    jc.doConfigure("./input/rolling/time1.xml", LogManager.getLoggerRepository());
    jc.dumpErrors();
    
    String datePattern = "yyyy-MM-dd_HH_mm_ss";

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[4];

    Calendar cal = Calendar.getInstance();

    for (int i = 0; i < 4; i++) {
      filenames[i] = "output/test1-" + sdf.format(cal.getTime());
      cal.add(Calendar.SECOND, 1);
    }

    System.out.println("Waiting until next second and 100 millis.");
    delayUntilNextSecond(100);
    System.out.println("Done waiting.");

    for (int i = 0; i < 5; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }

    for (int i = 0; i < 4; i++) {
      //System.out.println(i + " expected filename [" + filenames[i] + "].");
    }

    for (int i = 0; i < 4; i++) {
      assertTrue(Compare.compare(filenames[i], "witness/rolling/tbr-test1." + i));
    }
    
  }
  
  public void XXXtestWithJoran10() throws Exception {
    JoranConfigurator jc = new JoranConfigurator();
    jc.doConfigure("./input/rolling/time2.xml", LogManager.getLoggerRepository());
    jc.dumpErrors();
    
    String datePattern = "yyyy-MM-dd";

    SimpleDateFormat sdf = new SimpleDateFormat(datePattern);
    String[] filenames = new String[0];

    Calendar cal = Calendar.getInstance();

    filenames[0] = "output/test1-" + sdf.format(cal.getTime());

    for (int i = 0; i < 5; i++) {
      logger.debug("Hello---" + i);
      Thread.sleep(500);
    }


    for (int i = 0; i < 1; i++) {
      assertTrue(Compare.compare(filenames[i], "witness/rolling/tbr-test10." + i));
    }
    
  }
#endif  

  void delayUntilNextSecond(int millis) {
    apr_time_t now = apr_time_now();
    apr_time_t next = ((now / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC
          + millis * 1000L;

    apr_sleep(next - now);
  }

#if 0
  void delayUntilNextMinute(int seconds) {
    long now = System.currentTimeMillis();
    Calendar cal = Calendar.getInstance();
    cal.setTime(new Date(now));

    cal.set(Calendar.SECOND, seconds);
    cal.add(Calendar.MINUTE, 1);

    long next = cal.getTime().getTime();

    try {
      Thread.sleep(next - now);
    } catch (Exception e) {
    }
  }

  public static Test XXXsuite() {
    TestSuite suite = new TestSuite();

//    suite.addTest(new TimeBasedRollingTest("test1"));
//    suite.addTest(new TimeBasedRollingTest("test2"));
//    suite.addTest(new TimeBasedRollingTest("test3"));
//    suite.addTest(new TimeBasedRollingTest("test4"));
//
//    suite.addTest(new TimeBasedRollingTest("test5"));
//    suite.addTest(new TimeBasedRollingTest("test6"));
//    suite.addTest(new TimeBasedRollingTest("testWithJoran1"));
    suite.addTest(new TimeBasedRollingTest("testWithJoran10"));
    
    return suite;
  }
#endif
};


//LoggerPtr TimeBasedRollingTest::logger("org.apache.log4j.rolling.TimeBasedRollingTest");

#endif