--TEST--
Extending Semaphore
--SKIPIF--
<?php
include __DIR__ . '/../skipif.inc';
?>
--FILE--
<?php
use Win\System\Semaphore;
use Win\System\RuntimeException;

class goodSem extends Semaphore {}

class badSem extends Semaphore {
    public function __construct() {
        echo "bad";
    }
}

class argSemaphore extends Semaphore {
    public function __construct($name) {
        parent::__construct($name);
    }
}

// good semaphore is fine
$semaphore = new goodSem();
var_dump($semaphore->getName());

// bad semaphore will throw exception
try {
    $semaphore = new badSem();
} catch (RuntimeException $e) {
    echo $e->getMessage(), "\n";
}

// arg mutex will create a new named mutex
$semaphore = new argSemaphore('foobar');
var_dump($semaphore->getName());
?>
--EXPECT--
NULL
badparent::__construct() must be called in badSem::__construct()
string(6) "foobar"