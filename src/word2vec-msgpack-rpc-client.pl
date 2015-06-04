#!/usr/bin/env perl

use strict;
use warnings;
use utf8;
use YAML;
use AnyEvent::MPRPC;

my $client = mprpc_client '0.0.0.0', '22676';
{
    my $d = $client->call( distance => ['LINE'] );
    my $res = $d->recv;
    print Dump $res;
}

{
    my $a = $client->call( analogy => ["LINE", "渋谷", "はてな"] );
    my $res = $a->recv;
    print Dump $res;
}
