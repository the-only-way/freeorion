from logging import error
import re
from typing import Iterable, List, Tuple

normalization_dict = {
    'empire': 'empire_object',
    'int': 'number',
    'str': 'string',
    'float': 'floating_number',
    'object': 'obj',
    'IntBoolMap': 'int_bool_map',
    'IntDblMap': 'int_dbl_map',
    'universeObject': 'base_object',
    'meterType': 'meter_type',
    'bool': 'boolean',
    'StringVec': 'string_list',
    'shipDesign': 'ship_design',
    'universe': 'universe_object',
    'researchQueue': 'research_queue',
    'resPoolMap': 'res_pool',
    'productionQueue': 'production_queue',
    'diplomaticMessage': 'diplomatic_message',
    'ship': 'ship_object',
    'species': 'species_object',
    'planetType': 'planet_type',
    'system': 'system_object',
    'tech': 'tech_object',
    'list': 'item_list',
    'planet': 'planet_object',
    'shipPart': 'ship_part',
    'resPool': 'res_pool',
    'researchQueueElement': 'research_queue_element',
    'shipSlotType': 'ship_slot_type',
    'IntIntMap': 'int_int_map',
    'IntPairVec': 'int_pair_list',
    'IntSet': 'int_set',
    'IntSetSet': 'int_set_set',
    'IntVec': 'int_list',
    'IntVisibilityMap': 'int_visibility_map',
    'UnlockableItemVec': 'unlockable_item_vec',
    'StringSet': 'string_set',
    'VisibilityIntMap': 'visibility_int_map',
    'buildingType': 'buildingType',
    'productionQueueElement': 'production_queue_element',
    'resourceType': 'resource_type',
    'fleetAggression': 'fleet_aggression',
    'buildType': 'build_type',
    'field': 'field',
    'shipHull': 'ship_hull',
    'PlanetSize': 'planet_size',
    'planetSize': 'planet_size',
    'unlockableItemType': 'unlockable_item_type',
    'dict': 'dictionary',
    'StarType': 'star_type',
    'starType': 'star_type',
    'UnlockableItemType': 'unlocable_item_type',
    'FleetPlan': 'fleet_plan',
    'MeterTypeMeterMap': 'meter_type_meter_map',
    'PairIntInt_IntMap': 'pair_int_int_int_map',
    'MonsterFleetPlan': 'monster_fleet_plan',
    'ShipPartMeterMap': 'ship_part_meter_map',
    'ShipSlotVec': 'ship_slot_vec',
    'special': 'special',
    'IntFltMap': 'int_flt_map',
    'ruleType': 'rule_type',
    'influenceQueueElement': 'influence_queue_element'
}


def normalize_name(tp):
    argument_type, provided_name = tp
    if not provided_name.startswith('arg'):
        return provided_name

    if argument_type not in normalization_dict:
        error("Can't find proper name for: %s, please add it to name mapping \n" % argument_type)
        normalization_dict[argument_type] = 'arg'
    return normalization_dict[argument_type]


def get_argument_names(arguments, is_class):
    counts = {}
    names = []

    types = [x[0] for x in arguments]

    if is_class:
        arguments = arguments[1:]

    arg_names = [normalize_name(tp) for tp in arguments]
    for tp, arg_name in zip(arguments, arg_names):
        if arg_names.count(arg_name) == 1:
            suffix = ''
        else:
            if arg_name in counts:
                counts[arg_name] += 1
            else:
                counts[arg_name] = 1
            suffix = str(counts[arg_name])
        names.append('%s%s' % (arg_name, suffix))
    if is_class:
        names.insert(0, 'self')
    return names, types


def parse_name(txt):
    match = re.match(r'\w+\((.*)\) -> (.+) :', txt)
    args, return_type = match.group(1, 2)
    args = [x.strip(' (').split(')') for x in args.split(',') if x]
    return [x[0] for x in args], return_type


def _get_duplicated_arguments_message(name, raw_arg_types) -> Iterable[str]:
    yield ""
    yield "Cannot merge different set of arguments for a callable: %s" % name
    args = (', '.join('%s:%s' % (name, tp) for tp, name in arg_set) for arg_set in raw_arg_types)
    yield from ("   %s:  %s" % (i, args_line) for i, args_line in enumerate(args, start=1))
    yield ""


def merge_args(name: str, raw_arg_types: List[tuple], is_class: bool) -> Tuple[List[str], List[Tuple[str, str]]]:
    """
    Merge multiple set of arguments together.

    Single argument set is used as is.
    If we have two unique argument sets, and on of them is empty, use keywords.
    In other cases log error and use first one.
    """
    # If wrapper define functions that have same name, and same arguments but different return types,
    # it will come here with len(arg_types) >= 2, where all arguments set are the same.
    size = len(raw_arg_types)
    arg_types = sorted(set(raw_arg_types))
    if len(arg_types) != size:
        error("[%s] Duplicated argument types", name)

    if len(arg_types) == 1:
        names, types = get_argument_names(arg_types[0], is_class)
        use_keyword = False
    elif len(arg_types) == 2 and any(not x for x in arg_types):

        names, types = get_argument_names(next(filter(None, arg_types)), is_class)  # pylint: disable=filter-builtin-not-iterating
        use_keyword = True
    else:

        error("\n".join(_get_duplicated_arguments_message(name, raw_arg_types)))

        names, types = get_argument_names(raw_arg_types[0], is_class)
        use_keyword = False
    return ['%s=None' % arg_name for arg_name in names] if use_keyword else names, list(zip(names, types))


def normalize_rtype(rtype):
    if rtype == 'iterator':
        return 'iter'
    return rtype


class Docs:
    def __init__(self, text, indent, is_class=False):
        self.indent = indent
        self.is_class = is_class

        if not text:
            self.rtype = 'unknown'
            self.args = ['*args']
            self.header = ''
            return

        self.text = text

        lines = [x.strip() for x in self.text.split('\n')]

        def parse_signature(line):
            expre = re.compile(r'(\w+)\((.*)\) -> (\w+)')
            name, args, rtype = expre.match(line).group(1, 2, 3)
            args = tuple(re.findall(r'\((\w+)\) *(\w+)', args))
            return name, args, rtype

        res = []
        name, args, rtype = parse_signature(lines[0])
        res.append((args, rtype, []))
        for line in lines[1:]:
            if line.startswith('%s(' % name):
                name, args, rtype = parse_signature(line)
                res.append((args, rtype, []))
            else:
                res[-1][2].append(line)

        self.resources = res

        args, rtypes, infos = zip(*res)
        if len(set(rtypes)) != 1:
            error("[%s] Different rtypes", name)
        self.rtype = normalize_rtype(rtypes[0])

        # cut of first and last string if they are empty
        # we cant cut off all empty lines, because it can be inside docstring
        doc_lines = []

        for doc_part in infos:
            if not doc_part:
                continue
            else:
                # cut first and last empty strings
                if not doc_part[0]:
                    doc_part = doc_part[1:]
                if not doc_part:
                    continue
                if not doc_part[-1]:
                    doc_part = doc_part[:-1]
                doc_lines.append('\n'.join(doc_part))

        # if docs are equals show only one of them
        self.header = sorted(doc_lines)
        argument_declaration, args = merge_args(name, args, self.is_class)
        self.argument_declaration = argument_declaration
        self.args = args

    def get_argument_string(self):

        if self.is_class:
            args = ['self']
        else:
            args = []
        args.extend("%s: %s" % (arg_name, arg_type) for arg_name, arg_type in self.args[self.is_class:])
        return ', '.join(args)

    def get_doc_string(self):
        doc = []
        if self.header:
            doc.append('"""')
            if self.header:
                doc.extend(self.header)
            doc.append('"""')

        return '\n'.join('%s%s' % (' ' * 4 * self.indent if x else '', x) for x in doc)


if __name__ == '__main__':
    # example1 = """__delitem__( (IntBoolMap)arg1, (object)arg2) -> None"""
    example1 = """getEmpire() -> empire\n\ngetEmpire((int)star_name, (int)arg2, (int)arg3) -> empire"""

    # example1 = ("""getUserDataDir() -> str :\n
    #     Returns path to directory where FreeOrion stores user specific data (saves, etc.).
    #
    #     getUserDataDir((int)args1) -> str :\n
    #         Unicorns.
    #     """)
    #
    # example1 = """getUserDataDir() -> str :\n    Returns path to directory where FreeOrion stores user specific data (config files, saves, etc.)."""

    info = Docs(example1, 1)
    print("=" * 100)
    print("Arg string:", info.get_argument_string())
    print("=" * 100)
    print("Doc string:\n", info.get_doc_string())

    # double standards
    # canBuild ['empire', 'buildType', 'str', 'int'], ['empire', 'buildType', 'int', 'int']
    # inField ['field', 'universeObject'], ['field', 'float', 'float']
    # validShipDesign ['str', 'StringVec']
