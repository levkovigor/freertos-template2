#!/usr/bin/env python3
import argparse
from objects.objects import parse_objects
from events.event_parser import parse_events


def main():
    parser = argparse.ArgumentParser('Arguments for FSFW MOD generation')
    choices = ("events", "objects", "returnvalues", "subservices")
    parser.add_argument(
        'type', metavar='type', choices=choices,
        help=f'Type of MOD data to generate. Choices: {choices}'
    )
    args = parser.parse_args()
    if args.type == 'objects':
        parse_objects()
    elif args.type == 'events':
        parse_events()
    pass


if __name__ == "__main__":
    main()
