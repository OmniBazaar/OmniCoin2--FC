#include <fc/log/appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/thread/unique_lock.hpp>
#include <unordered_map>
#include <string>
#include <fc/thread/spin_lock.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/gelf_appender.hpp>
#include <fc/variant.hpp>
#include "console_defines.h"


namespace fc {

   std::unordered_map<std::string,appender::ptr>& get_appender_map() {
     static std::unordered_map<std::string,appender::ptr> lm;
     return lm;
   }
   std::unordered_map<std::string,appender_factory::ptr>& get_appender_factory_map() {
     static std::unordered_map<std::string,appender_factory::ptr> lm;
     return lm;
   }
   appender::ptr appender::get( const fc::string& s ) {
     static fc::spin_lock appender_spinlock;
      scoped_lock<spin_lock> lock(appender_spinlock);
      return get_appender_map()[s];
   }
   bool  appender::register_appender( const fc::string& type, const appender_factory::ptr& f )
   {
      get_appender_factory_map()[type] = f;
      return true;
   }
   appender::ptr appender::create( const fc::string& name, const fc::string& type, const variant& args  )
   {
      auto fact_itr = get_appender_factory_map().find(type);
      if( fact_itr == get_appender_factory_map().end() ) {
         //wlog( "Unknown appender type '%s'", type.c_str() );
         return appender::ptr();
      }
      auto ap = fact_itr->second->create( args );
      get_appender_map()[name] = ap;
      return ap;
   }

   string appender::get_omnibazaar_version()
   {
       // FC lib is both an individual project and a git submodule of OmniCoin2 project, and OC2 depends on FC,
       // so we can't have FC depend on OC2.
       // Thus, already available in OC2 'graphene::utilities::git_revision_sha' variable is not available to FC.
       // So, in order to propagate revision info from OC2 to FC, OC2 will globally define following preprocessor value,
       // and if FC is compiled as part of OC2 then it will include following lines of code and print OC2 node version.
#ifdef OMNIBAZAAR_GIT_REVISION_SHA
       return string(OMNIBAZAAR_GIT_REVISION_SHA).substr(0, 6);
#else
       return string();
#endif
   }
   
   static bool reg_console_appender = appender::register_appender<console_appender>( "console" );
   static bool reg_file_appender = appender::register_appender<file_appender>( "file" );
   static bool reg_gelf_appender = appender::register_appender<gelf_appender>( "gelf" );

} // namespace fc
