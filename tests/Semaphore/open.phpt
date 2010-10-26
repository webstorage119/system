--TEST--
Win\System\Semaphore::open() method
--SKIPIF--
<?php
if(!extension_loaded('winsystem')) die('skip - winsystem extension not available');
?>
--FILE--
<?php
use Win\System\Semaphore;
use Win\System\Event;
use Win\System\Unicode;
use Win\System\CodePage;
use Win\System\ArgumentException;
use Win\System\Exception;

// create new named semaphore
$semaphore = new Semaphore('foobar');

// open that semaphore
$semaphore = Semaphore::open('foobar');

// new unicode semaphore
$string = 'काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥';
$unicode = new Unicode($string, CodePage::UTF8);
$semaphore = new Semaphore($unicode);

// open unicode semaphore
$semaphore = Semaphore::open($unicode);

// non-existent semaphore
try {
    $semaphore = Semaphore::open('hello');
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}

// requires at least 1 arg
try {
    $mutex = Semaphore::open();
} catch (ArgumentException $e) {
    echo $e->getMessage(), "\n";
}

// requires 1-2 args, 3 is too many
try {
    $mutex = Semaphore::open('foobar', 1, 1);
} catch (ArgumentException $e) {
    echo $e->getMessage(), "\n";
}

// arg 1 must be stringable or instanceof Unicode
try {
    $mutex = Semaphore::open(array());
} catch (ArgumentException $e) {
    echo $e->getMessage(), "\n";
}

// arg 2 must be booleanable
try {
    $mutex = Semaphore::open('foobar', array());
} catch (ArgumentException $e) {
    echo $e->getMessage(), "\n";
}
?>
--EXPECTF--
Semaphore was not found and could not be opened
Win\System\Semaphore::open() expects at least 1 parameter, 0 given
Win\System\Semaphore::open() expects at most 2 parameters, 3 given
Win\System\Semaphore::open() expects parameter 1 to be string, array given
Win\System\Semaphore::open() expects parameter 2 to be boolean, array given