* Redis db will sync to 'dump.rdb' periodically, can be forced by SAVE or BGSAVE commands. 
  (For BGSAVE need to poll LASTSAVE to check for completion)

* These rdb can be parsed to json using rdb-cli: https://github.com/redis/librdb
  OR, can loaded using a redis instance pointing to it (refer redis.conf)
