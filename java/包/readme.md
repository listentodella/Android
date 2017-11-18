# javac A.java//将在当前目录下生成A.class
# 能否把这些class文件存放于其他目录呢？

# 包实际上是一个文件夹（可以含有多级子目录）

### package 包名称.子包名称;
### 编译命令：
    javac -d <dir> <file.java>（表示在<dir>目录下生成包）
 作用：解决类的同名冲突问题
 ————这样大型项目就不会因为同名问题烦恼了

# import
    使用不同包的类时需要import
    public class可以被外包访问；class只能在本包中访问
    import 包名称.子包名称.类名称;//手工导入某个类
    import 包名称.子包名称.*;//由JVM自动加载所需要的类
    不同名的类不需要加上包区分


    jar
    jar -help
    jar -cf my.jar 目录：打包
    jar -cvf my.jar 目录：可以在压缩的时候输出信息



# CLASSPATH
## 作用1：编译的时候查找源代码
## 作用2：运行的时候查找class文件或者压缩包
      设置方法：export CLASSPATH=<dir1>:<dir2>:...:<file1.jar>:<file2.jar>:...

例如
      export CLASSPATH=.:my.jar
      再javac 编译
