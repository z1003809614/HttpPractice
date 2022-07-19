# 数据库模块

## 模块设计

1. Class SQLite3
   1. 对应一个实体数据库
   2. 内部封装了sqlite3_exec等对应的库函数，可以执行SQL语句
2. Class SQLite3Stmt
   1. 对应一个SQL的预处理对象
   2. 内部封装SQLite3的bind相关库函数，实现预处理情况下的SQL语句的执行
3. Class SQLite3Data
   1. 内部执行SQLite3Stmt对象
   2. 由于该数据库的执行SQL语句后的数据信息其实是存储在stmt预处理对象中，所以该类内部是需要指向一个SQLite3Stmt对象的；
4. Class SQLite3Transaction
   1. 封装了事务的核心执行逻辑
   2. beging(): 在SQLite3中，即执行 SQL= "BEGIN"就表示事务开始；
   3. commit(): 在SQLite3中，即执行 SQL= "COMMIT"就表示事务提交；
   4. rollback(): 在SQLite3中，即执行 SQL= "ROLLBACK"就表示事务回滚；
5. Class SQLite3Manager
   1. 内部保存SQLite3实例
   2. 并提供一些方法，能够直接对内部存储的数据库执行SQL语句；

## 执行逻辑
```
                                    创建数据库db
                                        |
                                        V
            调用db->execute(sql)执行sql语句  or  创建Stmt(db,sql)对象
                                                        |
                                                        V
                                                调用Stmt->bing(value)
                                                        |
                                                        V
                                            调用Stmt->execut()执行即可完成
```

## 知识点

1. 数据库预处理
   1. 即时SQL 
      1. 一条 SQL 在 DB 接收到最终执行完毕返回，大致的过程如下：
      2. 词法和语义解析；
      3. 优化 SQL 语句，制定执行计划；
      4. 执行并返回结果；
      5. 如上，一条 SQL 直接是走流程处理，一次编译，单次运行，此类普通语句被称作 Immediate Statements （即时 SQL）。
   2. 预处理 SQL
      1. 绝大多数情况下，某需求某一条 SQL 语句可能会被反复调用执行，或者每次执行的时候只有个别的值不同（比如 select 的 where 子句值不同，update 的 set 子句值不同，insert 的 values 值不同）。如果每次都需要经过上面的词法语义解析、语句优化、制定执行计划等，则效率就明显不行了。
      2. 所谓预编译语句就是将此类 SQL 语句中的值用占位符替代，可以视为将 SQL 语句模板化或者说参数化，一般称这类语句叫Prepared Statements。
      3. 预编译语句的优势在于归纳为：一次编译、多次运行，省去了解析优化等过程；此外预编译语句能防止 SQL 注入。
      4. 预处理编译 SQL 是占用资源的，所以在使用后注意及时使用 DEALLOCATE PREPARE 释放资源，这是一个好习惯
2. 事务简介
   1. 一个最小的不可再分的工作单元；通常一个事务对应一个完整的业务(例如银行账户转账业务，该业务就是一个最小的工作单元)，同时这个完整的业务需要执行多次的DML(insert、update、delete)语句共同联合完成。A转账给B，这里面就需要执行两次update操作。
   2. 在MySQL中只有使用了Innodb数据库引擎的数据库或表才支持事务。事务处理可以用来维护数据库的完整性，保证成批的SQL语句要么全部执行，要么全部不执行
   3. 事务满足四个条件
      1. 原子性（Atomicity，或称不可分割性）；一个事务（transaction）中的所有操作，要么全部完成，要么全部不完成，不会结束在中间某个环节。事务在执行过程中发生错误，会被回滚（Rollback）到事务开始前的状态，就像这个事务从来没有执行过一样
      2. 一致性（Consistency）、在事务开始之前和事务结束以后，数据库的完整性没有被破坏。这表示写入的资料必须完全符合所有的预设规则，这包含资料的精确度、串联性以及后续数据库可以自发性地完成预定的工作。
      3. 隔离性（Isolation，又称独立性）、数据库允许多个并发事务同时对其数据进行读写和修改的能力，隔离性可以防止多个事务并发执行时由于交叉执行而导致数据的不一致。事务隔离分为不同级别，包括读未提交（Read uncommitted）、读提交（read committed）、可重复读（repeatable read）和串行化（Serializable）。
      4. 持久性（Durability）、事务处理结束后，对数据的修改就是永久的，即便系统故障也不会丢失。
3. SQL注入问题
   1. sql注入性问题：所谓SQL注入式攻击，就是攻击者把SQL命令插入到Web表单的输入域或页面请求的查询字符串，欺骗服务器执行恶意的SQL命令。在某些表单中，用户输入的内容直接用来构造（或者影响）动态SQL命令，或作为存储过程的输入参数，这类表单特别容易受到SQL注入式攻击。
```
代码举例：
<%@ page language="java" import="java.util.*" pageEncoding="gb2312"%>
<html>
<body>
<%    
    //获得账号和密码　　　　
    String account=request.getParament("account");　　　　
    String password=request.getParament("password");　　　　
    if(account!=null){　　　　　　
        //验证账号和密码　　　　　　　　
        String  sql="SELECT * FROM T_CUSTOMER WHRER ACCOUNT= "+ account + " AND PASSWORD = "+ password;
        Out.println("数据库执行语句：<br>"+sql);       
    }
%>
</body>
</html>

假如用户输入的账号为：Java ' or  1=1  -- , 密码任意输入
SELECT * FROM T_CUSTOMER WHRER ACCOUNT= 'Java' or 1=1 --' AND PASSWORD = '123';（注意：--（杠杠空格）表示注释，不会被运行）
若此时数据库对该sql进行验证时，因为“ 1=1” 永远成立，所以数据库将返回表中的信息。此时，网站就受到了sql注入的攻击。
```