// [kit patch: LegacyScriptEngine] python-markdown compat: a block after a blank line attaches to the item at
// depth floor(indent / 4) - 1 (Python-Markdown's ListIndentProcessor), like the live MkDocs site.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import { preprocess } from '../docs/node/markdown/preprocess.ts';

const md = (src) => unified().use(remarkParse).parse(preprocess(src, { compat: 'python-markdown' }));
const shape = (n) =>
  n.type === 'list' || n.type === 'listItem' || n.type === 'root'
    ? { [n.type]: n.children.map(shape) }
    : n.type === 'paragraph'
      ? 'p:' + n.children.map((c) => c.value ?? c.children?.map((x) => x.value).join('') ?? '').join('').split('\n')[0]
      : n.type;

test('6-space paragraph after a nested item belongs to the outer item (LSE Entity.teleport)', () => {
  const tree = md(
    [
      '- Parameters:',
      '    - pos : `IntPos`  ',
      '      Target position',
      '',
      '    - rot: `DirectionAngle`',
      '',
      '      (Optional) The orientation',
      '- Return value: Whether',
      '- Return value type:  `Boolean`',
    ].join('\n'),
  );
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:pos : IntPosTarget position'] }, { listItem: ['p:rot: DirectionAngle'] }] },
              'p:(Optional) The orientation',
            ],
          },
        ],
      },
    ],
  });
});

test('items after a hoisted paragraph start a new nested list; the next hoist returns to the outer item', () => {
  const tree = md(
    [
      '- Parameters:',
      '    - a: `Integer`',
      '',
      '      (optional) A',
      '',
      '    - b: `Integer`',
      '',
      '      (optional) B',
      '',
      '- Return value: x',
    ].join('\n'),
  );
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:a: Integer'] }] },
              'p:(optional) A',
              { list: [{ listItem: ['p:b: Integer'] }] },
              'p:(optional) B',
            ],
          },
          { listItem: ['p:Return value: x'] },
        ],
      },
    ],
  });
});

test('8-space paragraph stays in the nested item', () => {
  const tree = md(['- Parameters:', '    - rot: `DirectionAngle`', '', '        (Optional) The orientation'].join('\n'));
  assert.deepEqual(shape(tree), {
    root: [
      {
        list: [
          {
            listItem: [
              'p:Parameters:',
              { list: [{ listItem: ['p:rot: DirectionAngle', 'p:(Optional) The orientation'] }] },
            ],
          },
        ],
      },
    ],
  });
});

// The expected shapes below were checked against Python-Markdown 3.11.1.
const optionalA = (outer) =>
  md([`${outer} Parameters:`, '    - a: x', '', '      (Optional) A', '', '      More about A'].join('\n'));
const optionalAShape = {
  root: [{ list: [{ listItem: ['p:Parameters:', { list: [{ listItem: ['p:a: x'] }] }, 'p:(Optional) A', 'p:More about A'] }] }],
};

test('a second block after a re-attached paragraph stays a paragraph of the same item (bullet outer)', () => {
  assert.deepEqual(shape(optionalA('-')), optionalAShape);
});

test('a second block after a re-attached paragraph stays a paragraph of the same item (ordered outer)', () => {
  assert.deepEqual(shape(optionalA('1.')), optionalAShape);
});

test('depth-2 re-attachment: further 10-space blocks are paragraphs, 14-space blocks are code', () => {
  const src = (last) => ['- L0', '    - L1', '        - L2: x', '', '          (Optional) A', '', last].join('\n');
  const item = (tail) => ({
    root: [
      {
        list: [
          {
            listItem: [
              'p:L0',
              { list: [{ listItem: ['p:L1', { list: [{ listItem: ['p:L2: x'] }] }, 'p:(Optional) A', tail] }] },
            ],
          },
        ],
      },
    ],
  });
  assert.deepEqual(shape(md(src('          More about A'))), item('p:More about A'));
  assert.deepEqual(shape(md(src('              code here'))), item('code'));
});

test('a 6-space block after a top-level item is a paragraph, not code (Python-Markdown strips 4 spaces)', () => {
  const expected = { root: [{ list: [{ listItem: ['p:a', 'p:text'] }] }] };
  assert.deepEqual(shape(md(['- a', '', '      text'].join('\n'))), expected);
  assert.deepEqual(shape(md(['1. a', '', '      text'].join('\n'))), expected);
});

test('the re-attachment shift applies only to the lines of the re-attached block', () => {
  const pre = preprocess(
    ['- P:', '    - a: x', '', '      (Optional) A', '      continued', '', '    - b: y', '      lazy line'].join('\n'),
    { compat: 'python-markdown' },
  ).split('\n');
  // the re-attached block (content column 2 + the 2 spaces Python-Markdown keeps) and its lazy line move together ...
  assert.deepEqual(pre.slice(3, 5), ['    (Optional) A', '    continued']);
  // ... but a later item and its lazy line keep their indentation
  assert.deepEqual(pre.slice(6, 8), ['    - b: y', '      lazy line']);
});
